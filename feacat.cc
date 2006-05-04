#include "io.hh"
#include "conf.hh"
#include "FeatureGenerator.hh"

conf::Config config;
FeatureGenerator gen;
bool raw_output = false;

void
print_feature(const FeatureVec &fea)
{
  // Raw output
  if (raw_output) {
    for (int i = 0; i < fea.dim(); i++) {
      float tmp = fea[i];
      fwrite(&tmp, sizeof(float), 1, stdout);
    }
  }

  // ASCII output
  else {
    for (int i=0; i < fea.dim(); i++)
      printf("%8.2f ", fea[i]);
    printf("\n");
  }
}

int
main(int argc, char *argv[])
{
  assert(sizeof(float) == 4);

  try {
    config("usage: feacat [OPTION...] FILE\n")
      ('h', "help", "", "", "display help")
      ('c', "config=FILE", "arg must", "", "read feature configuration")
      ('w', "write-config=FILE", "arg", "", "write feature configuration")
      ('r', "raw-input", "", "", "raw audio input")
      ('R', "raw-output", "", "", "raw float output")
      ('s', "start-frame=INT", "arg", "", "audio start frame")
      ('e', "end-frame=INT", "arg", "", "audio end frame")
      ;
    config.default_parse(argc, argv);
    if (config.arguments.size() != 1)
      config.print_help(stderr, 1);
    raw_output = config["raw-output"].specified;

    gen.load_configuration(io::Stream(config["config"].get_str()));
    gen.open(config.arguments[0], config["raw-input"].specified);

    if (config["write-config"].specified)
      gen.write_configuration(io::Stream(config["write-config"].get_str(),
                                         "w"));

    int start_frame = 0;
    int end_frame = INT_MAX;
    if (config["start-frame"].specified)
      start_frame = config["start-frame"].get_int();
    if (config["end-frame"].specified)
      end_frame = config["end-frame"].get_int();
    for (int f = start_frame; f < end_frame; f++) {
      const FeatureVec fea = gen.generate(f);
      if (gen.eof())
	break;
      print_feature(fea);
    }

    gen.close();
  }
  catch (std::exception &e) {
    fprintf(stderr, "exception: %s\n", e.what());
    abort();
  }
  catch (std::string &str) {
    fprintf(stderr, "exception: %s\n", str.c_str());
    abort();
  }
}