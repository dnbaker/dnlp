#include "parse.h"

using namespace dnlp;

int main(int argc, char *argv[]) {
    ks::string ks;
    int n;
    if(argc > 1) {
        std::FILE *fp = std::fopen(argv[1], "rb");
        size_t fsz = filesize(argv[1]);
        for(int c; ((c = std::fgetc(fp)) != EOF);ks.putc_(c));
        ks.terminate();
        std::fclose(fp);
        n = argc > 2 ? std::atoi(argv[2]): 2;
    } else ks.puts("hello world I am cutting things into pieces for producing stuff"), n = 2;
    ASCIITextSpacer spacer(ks.data(), ks.size());
    NGrammer<> ng(n, true);
    ng.for_each([&](auto &x) {
        LOG_DEBUG("Size of x: %zu\n", x.size());
        auto it = x.begin();
        size_t i = 0;
        ks::string ks2(it->data());
        ks2.putc_(':');
        ks2.putc_('0');
        ks2.putc_(',');
        while(++it != x.end()) {
            auto &el = *it;
            ks2.putsn_(el.data(), el.size());
            ks2.putc_(':');
            ks2.sprintf("%zu", ++i);
            ks2.putc_(',');
        }
        ks2.putc('|');
        LOG_DEBUG("str is now '%s'\n", ks2.data());
        i = 0;
        for(auto &el: x) {
            std::fprintf(stderr, "string at ind %zu is '%s'\n", i++, el.data());
        }
        ks2.flush(stdout);
    }, spacer);
}
