#ifndef EXPANDER_HH
#define EXPANDER_HH

#include <vector>

#include "Lexicon.hh"
#include "Hmm.hh"
#include "Acoustics.hh"

class Expander {
public:
  // FIXME! Would it be much faster to use shorts and floats here?
  class Word {
  public:
    Word() : 
      avg_log_prob(0), log_prob(0), frames(-1), word_id(-1), active(false) { }
    float avg_log_prob;
    float log_prob;
    int frames;
    int word_id;
    bool active;
  };

  struct WordCompare {
    inline bool operator()(const Word *a, const Word *b) {
      return a->avg_log_prob > b->avg_log_prob;
    }
  };

  // Actions
  Expander(const std::vector<Hmm> &hmms, Lexicon &lexicon,
	   Acoustics &m_acoustics);
  inline void get_settings(const Expander &expander)
    {
      m_forced_end = expander.m_forced_end;
      m_token_limit = expander.m_token_limit;
      m_beam = expander.m_beam;
      m_max_state_duration = expander.m_max_state_duration;
    }
  void sort_best_tokens(int tokens);
  void keep_best_tokens(int tokens);
  void move_all_tokens();
  void clear_tokens();
  void create_initial_tokens(int start_frame);
  void expand(int start_frame, int frames);

  // Options
  inline void set_forced_end(bool forced_end) { m_forced_end = forced_end; }
  inline void set_token_limit(int limit) { m_token_limit = limit; }
  inline void set_beam(float beam) { m_beam = beam; }
  inline void set_max_state_duration(int duration) 
    { m_max_state_duration = duration; }

  // Info
  inline std::vector<Lexicon::Token*> &tokens() { return m_tokens; }
  inline std::vector<Word*> &words() { return m_sorted_words; }
  inline int eof_frame() const { return m_acoustics.eof_frame(); }

  // Debug
  void debug_print_history(Lexicon::Token *token);
  void debug_print_timit(Lexicon::Token *token);
  void debug_print_tokens();

private:
  Lexicon::Token *token_to_state(const Lexicon::Token *source_token,
				 Lexicon::State &source_state,
				 Lexicon::State &target_state,
				 float new_log_prob);

  const std::vector<Hmm> &m_hmms;
  Lexicon &m_lexicon;
  Acoustics &m_acoustics;

  // Options
  bool m_forced_end;
  int m_token_limit;
  float m_beam;
  int m_max_state_duration;

  // State
  std::vector<Lexicon::Token*> m_tokens;
  std::vector<Word> m_words;
  std::vector<Word*> m_sorted_words;
  int m_frame; // Current frame relative to the start frame.
  int m_frames; // Max frames per word
  float m_beam_best;
  float m_beam_best_tmp;
};

#endif /* EXPANDER_HH */
