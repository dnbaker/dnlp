#include "parse.h"

using namespace dnlp;

int main() {
    const char text []{"hello world I am cutting things into pieces for producing stuff"};
    std::fprintf(stderr, "Making spacer");
    ASCIITextSpacer spacer(text, sizeof(text) - 1);
    std::fprintf(stderr, "made spacer\n");
    NGrammer<> ng(2, true);
    std::fprintf(stderr, "made ng\n");
    ng.for_each([&](auto &x) {
        for(const auto &el: x) {
            std::fprintf(stderr, "String is %s\n", el.data());
        }
    }, spacer);
}
