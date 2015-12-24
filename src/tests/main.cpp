#include <gtest/gtest.h>

#include <openssl/err.h>

class initialize {
    public:
        initialize() {
            ::ERR_load_crypto_strings();
        }

        ~initialize() {
            ::ERR_free_strings();
        }
};

GTEST_API_ int main(int argc, char **argv) {
    initialize everything;

    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

