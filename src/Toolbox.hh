#ifndef TOOLBOX_HH
#define TOOLBOX_HH

#include "NowayHmmReader.hh"
#include "NowayLexiconReader.hh"
#include "LnaReaderCircular.hh"
#include "ArpaNgramReader.hh"
#include "Expander.hh"
#include "Search.hh"

class Toolbox {
public:
  Toolbox();
  
  void hmm_read(const char *file);
  void lex_read(const char *file);
  const std::string &lex_word() const { return m_lexicon_reader.word(); }
  const std::string &lex_phone() const { return m_lexicon_reader.phone(); }

  void ngram_read(const char *file);
  int ngram_lineno() const { return m_ngram_reader.lineno(); }

  void lna_open(const char *file, int models, int size);
  void lna_close();
  void lna_seek(int frame) { m_lna_reader.seek(frame); }

  void expand(int frame, int frames) { m_expander.expand(frame, frames); }
  void print_best();
  void print_words(int words);

  void init(int expand_window, int stacks, int reserved_hypos) 
    { 
      m_search.init_search(expand_window, stacks, reserved_hypos); 
    }
  void sort(int frame, int top = 0) { m_search.sort_stack(frame, top); }
  bool expand_stack(int frame) { return m_search.expand(frame); }
  void move_buffer(int frame) { m_search.move_buffer(frame); }
  void go(int frame) { m_search.go(frame); }
  bool run() { return m_search.run(); }
  bool runto(int frame);

  int first_frame() { return m_search.first_frame(); }
  int last_frame() { return m_search.last_frame(); }
  HypoStack &stack(int frame) { return m_search.stack(frame); }
  void prune(int frame, int top);
  int paths() const { return HypoPath::count; }

  void set_forced_end(bool forced_end) 
    { m_expander.set_forced_end(forced_end); }
  void set_hypo_limit(int hypo_limit) { m_search.set_hypo_limit(hypo_limit); } 
  void set_word_limit(int word_limit) { m_search.set_word_limit(word_limit); }
  void set_word_beam(double word_beam) { m_search.set_word_beam(word_beam); }
  void set_lm_scale(double lm_scale) { m_search.set_lm_scale(lm_scale); }
  void set_lm_offset(double lm_offset) { m_search.set_lm_offset(lm_offset); }
  void set_token_limit(int limit) { m_expander.set_token_limit(limit); }
  void set_state_beam(double beam) { m_expander.set_beam(beam); }
  void set_hypo_beam(double beam) { m_search.set_beam(beam); }
  void set_global_beam(double beam) 
    { m_search.set_global_beam(beam); }
  void set_max_state_duration(int duration) 
    { m_expander.set_max_state_duration(duration); }
  void set_verbose(bool verbose) { m_search.set_verbose(verbose); }

  void print_hypo(Hypo &hypo);

  struct OpenError : public std::exception {
    virtual const char *what() const throw()
      { return "Toolbox: open error"; }
  };

private:
  NowayHmmReader m_hmm_reader;
  const std::map<std::string,int> &m_hmm_map;
  const std::vector<Hmm> &m_hmms;

  NowayLexiconReader m_lexicon_reader;
  Lexicon &m_lexicon;
  const Vocabulary &m_vocabulary;

  LnaReaderCircular m_lna_reader;

  ArpaNgramReader m_ngram_reader;
  const Ngram &m_ngram;

  Expander m_expander;
  Search m_search;
};

#endif /* TOOLBOX_HH */
