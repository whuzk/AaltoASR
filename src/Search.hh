#ifndef SEARCH_HH
#define SEARCH_HH

#include <vector>
#include <deque>

#include <float.h>

#include "Ngram.hh"
#include "Expander.hh"
#include "Vocabulary.hh"

class HypoPath {
public:
  inline HypoPath(int word_id, int frame, HypoPath *prev);
  inline ~HypoPath();
  inline void link() { m_reference_count++; }
  inline static void unlink(HypoPath *path);
  inline int count() const { return m_reference_count; }
  inline bool guard() { return prev == NULL; }
  int word_id;
  int frame;
  HypoPath *prev;
  float lm_log_prob;
  float ac_log_prob;
  static int g_count;
private:
  int m_reference_count;
};

HypoPath::HypoPath(int word_id, int frame, HypoPath *prev)
  : word_id(word_id), frame(frame), prev(prev),
    lm_log_prob(0), ac_log_prob(0), m_reference_count(0)
{
  if (prev)
    prev->link();
  g_count++;
}

HypoPath::~HypoPath()
{
  g_count--;
}

void
HypoPath::unlink(HypoPath *path)
{
  if (path == NULL)
    return;
  while (path->m_reference_count == 1) {
    HypoPath *prev = path->prev;
    delete path;
    path = prev;
    if (!path)
      return;
  }
  path->m_reference_count--;
}

class Hypo {
public:
  inline Hypo();
  inline Hypo(int frame, float log_prob, HypoPath *path);
  inline Hypo(const Hypo &t);
  inline Hypo &operator=(const Hypo &hypo);
  inline ~Hypo();
  inline void add_path(int word_id, int frame);
  bool operator<(const Hypo &h) const { return log_prob > h.log_prob; }

  int frame;
  float log_prob;
  HypoPath *path;
};


Hypo::Hypo()
  : frame(0), log_prob(0), path(NULL)
{
}

Hypo::Hypo(int frame, float log_prob, HypoPath *path)
 : frame(frame), log_prob(log_prob), path(path)
{
  if (path)
    path->link();
}

Hypo::Hypo(const Hypo &h)
  : frame(h.frame),
    log_prob(h.log_prob),
    path(h.path)
{
  if (path)
    path->link();
}

Hypo&
Hypo::operator=(const Hypo &h)
{
  if (this == &h)
    return *this;

  HypoPath *old_path = path;

  frame = h.frame;
  log_prob = h.log_prob;
  path = h.path;

  if (path)
    path->link();

  if (old_path)
    HypoPath::unlink(old_path);

  return *this;
}

Hypo::~Hypo()
{
  if (path)
    HypoPath::unlink(path);
}

void
Hypo::add_path(int word_id, int frame)
{
  HypoPath *old_path = path;
  path = new HypoPath(word_id, frame, path);
  path->link();
  if (old_path)
    HypoPath::unlink(old_path);
}

class HypoStack : private std::vector<Hypo> {
public:
  inline HypoStack() : m_best_log_prob(-1e10) { }

  // Inherited from vector
  inline Hypo &operator[](int index) 
    { return std::vector<Hypo>::operator[](index); }
  inline Hypo &at(int index) { return std::vector<Hypo>::operator[](index); }
  inline size_type size() const { return std::vector<Hypo>::size(); }
  inline void reserve(int size) { std::vector<Hypo>::reserve(size); }
  inline bool empty() const { return std::vector<Hypo>::empty(); }

  // New functions
  inline void add(const Hypo &hypo);
  inline void partial_sort(int top);
  inline void sort();
  inline void prune(int top); // does not sort if not necessary
  inline void clear();
  void prune_similar(int length);

  // Status
  inline void reset_best() { m_best_log_prob = -1e10; m_best_index = -1; }
  inline float best_log_prob() const { return m_best_log_prob; }
  inline int best_index() const { return m_best_index; }

private:
  float m_best_log_prob;
  int m_best_index;
};

void
HypoStack::add(const Hypo &hypo)
{ 
  if (hypo.log_prob > m_best_log_prob) {
    m_best_log_prob = hypo.log_prob;
    m_best_index = size();
  }
  std::vector<Hypo>::push_back(hypo);
}

void
HypoStack::partial_sort(int top)
{
  if (top == 0 || top >= size()) {
    top = size();
    sort();
  }
  else {
    std::partial_sort(begin(), begin() + top, end());
    m_best_index = 0;
  }
}

void
HypoStack::sort()
{
  std::sort(begin(), end());
  m_best_index = 0;
}

// Assumes sort
void
HypoStack::prune(int top)
{
  if (top <= 0)
    clear();
  else if (top < size())
    resize(top);
}

void
HypoStack::clear()
{ 
  std::vector<Hypo>::clear(); 
  reset_best();
}

class Search {
public:

  Search(Expander &expander, const Vocabulary &vocabulary, 
	 const Ngram &ngram);

  // Debug and print
  void print_prunings();
  void print_path(HypoPath *path);
  void print_hypo(Hypo &hypo);
  void print_sure();

  // Operate
  void reset_search(int start_frame);
  void init_search(int expand_window, int stacks, int reserved_hypos);
  void sort_stack(int frame, int top = 0);
  bool expand(int frame);
  void move_buffer(int frame);
  void go(int frame);
  bool run();
  bool recognize_segment(int start_frame, int end_frame);
  void prune_similar(int frame, int length);

  // Info
  inline int frame() const { return m_frame; }
  inline int first_frame() const { return m_first_frame; }
  inline int last_frame() const { return m_last_frame; }
  int frame2stack(int frame) const;
  inline HypoStack &stack(int frame) { return m_stacks[frame2stack(frame)]; }
  inline const HypoStack &stack(int frame) const
    { return m_stacks[frame2stack(frame)]; }

  // Options
  void set_end_frame(int end_frame) { m_end_frame = end_frame; }

  void set_hypo_limit(int hypo_limit) { m_hypo_limit = hypo_limit; }
  void set_word_limit(int word_limit) { m_word_limit = word_limit; }
  void set_word_beam(float word_beam) { m_word_beam = word_beam; }
  void set_lm_scale(float lm_scale) { m_lm_scale = lm_scale; }
  void set_lm_offset(float lm_offset) { m_lm_offset = lm_offset; }
  void set_unk_offset(float unk_offset) { m_unk_offset = unk_offset; }
  void set_prune_similar(int prune_similar) { m_prune_similar = prune_similar; }
  void set_beam(float beam) { m_beam = beam; }
  void set_global_beam(float beam) { m_global_beam = beam; }
  void set_verbose(int verbose) { m_verbose = verbose; }
  void set_print_probs(bool print_probs) { m_print_probs = print_probs; }
  void set_print_indices(bool print_indices) { m_print_indices = print_indices; }

  // Exceptions
  struct ForgottenFrame : public std::exception {
    virtual const char *what() const throw()
      { return "Search: forgotten frame"; }
  };

  struct FutureFrame : public std::exception {
    virtual const char *what() const throw()
      { return "Search: future frame"; }
  };

private:
  void circulate(int &stack);

  Expander &m_expander;
  const Vocabulary &m_vocabulary;
  const Ngram &m_ngram;

  // Mapping between lexicon and language model
  std::vector<int> m_lex2lm;
  
  // State
  int m_frame;

  // Stacks
  int m_first_frame;	
  int m_last_frame;
  int m_first_stack;
  std::vector<HypoStack> m_stacks;
  int m_last_hypo_frame;

  // options
  int m_expand_window;	
  int m_end_frame;
  float m_lm_scale;
  float m_lm_offset;
  float m_unk_offset;
  int m_verbose;
  bool m_print_probs;
  bool m_print_indices;
  HypoPath *m_last_printed_path;

  // Pruning options
  int m_word_limit;	// How many best words are expanded
  float m_word_beam;   // Do not expand words outside this beam
  int m_prune_similar;  // Prune similar N-word endings 
  int m_hypo_limit;	// How many best hypos in a stack are expanded
  float m_beam;

  // Global pruning
  float m_global_best;
  float m_global_beam;
  int m_global_frame;

  // Pruning statistics
  int m_stack_expansions;
  int m_hypo_insertions;
  int m_limit_prunings;
  int m_beam_prunings;
  int m_similar_prunings;

  // Temporary variables
  std::deque<int> m_history;
};

#endif /* SEARCH_HH */
