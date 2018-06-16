#include "sketch.h"
#include "hll/hll.h"

using namespace dnlp;

int main(int argc, char *argv[]) {
    int n = 2, c, sketch_size = 16;
    uint64_t clhashseed1 = 137;
    std::string path = "/dev/urandom";
    while((c = getopt(argc, argv, "n:h:s:p:")) >= 0) {
        switch(c) {
            case 'n': n = std::atoi(optarg); break;
            case 's': clhashseed1 = std::strtoull(optarg, nullptr, 10); break;
            case 'p': sketch_size = std::atoi(optarg); break;
        }
    }
    if(optind < argc) path = argv[optind];
    if(optind < argc + 1) 
    std::FILE *fp = optind < argc + 1 ? std::fopen(argv[optind + 1], "w"): stdout;
    auto news = make_sketch<hll::hll_t>(path.data(), UNCOMPRESSED, n, clhasher(clhashseed1, 777), sketch_size);
    news.sum();
    news.write(fp);
    std::fprintf(stderr, "cardinality, appx: %lf\n", news.report());
    if(fp != stdout) std::fclose(fp);
}
