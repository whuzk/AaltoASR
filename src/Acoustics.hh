#ifndef ACOUSTICS_HH
#define ACOUSTICS_HH

#include <vector>

class Acoustics {
public:
  inline Acoustics() : m_log_prob(NULL), m_eof_frame(-1), m_num_models(0) { }
  virtual ~Acoustics() { }

  /** Go to specified frame.  Returns false if frame is past end of file.
   *
   * Note that some derived classes preserve only a fixed amount of
   * previous frames, because they support pipes.  Exception is thrown
   * in that case.
   **/
  virtual bool go_to(int frame) = 0;

  inline float log_prob(int model) const { return m_log_prob[model]; }
protected:
  float *m_log_prob;
  int m_num_models;
};

#endif /* ACOUSTICS_HH */
