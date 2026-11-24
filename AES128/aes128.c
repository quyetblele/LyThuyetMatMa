#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef uint8_t byte;

static byte mul(byte a, byte b) {
    byte r = 0;
    while (b) {
        if (b & 1) r ^= a;
        a = (byte)((a << 1) ^ ((a & 128) ? 27 : 0));
        b >>= 1;
    }
    return r;
}

static byte rot(byte x, int n) {
    return (byte)((x << n) | (x >> (8 - n)));
}

static byte sub(byte x) {
    byte y = 1, a = x;
    for (int n = 254; n; n >>= 1) {
        if (n & 1) y = mul(y, a);
        a = mul(a, a);
    }
    return y ^ rot(y, 1) ^ rot(y, 2) ^ rot(y, 3) ^ rot(y, 4) ^ 0x63;
}

static void print(const char *label, const byte *s) {
    printf("%-14s ", label);
    for (int i = 0; i < 16; i++) printf("%02X", s[i]);
    puts("");
}

static void expand(const byte *key, byte *w) {
    memcpy(w, key, 16);
    byte rc = 1, t[4];
    for (int i = 16; i < 176; i += 4) {
        memcpy(t, w + i - 4, 4);
        if (i % 16 == 0) {
            byte a = t[0];
            t[0] = sub(t[1]) ^ rc;
            t[1] = sub(t[2]);
            t[2] = sub(t[3]);
            t[3] = sub(a);
            rc = mul(rc, 2);
        }
        for (int j = 0; j < 4; j++) w[i+j] = w[i+j-16] ^ t[j];
    }
}

static void add(byte *s, const byte *k) {
    for (int i = 0; i < 16; i++) s[i] ^= k[i];
}

static void shift(byte *s) {
    byte t[16];
    memcpy(t, s, 16);
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            s[4*c+r] = t[4*((c+r)%4)+r];
}

static void mix(byte *s) {
    for (int c = 0; c < 4; c++) {
        byte *p = s + 4*c, a[4];
        memcpy(a, p, 4);
        for (int r = 0; r < 4; r++)
            p[r] = mul(a[r],2) ^ mul(a[(r+1)%4],3) ^ a[(r+2)%4] ^ a[(r+3)%4];
    }
}

static int parse(const char *h, byte *s) {
    if (strlen(h) != 32 || strspn(h, "0123456789abcdefABCDEF") != 32) return 0;
    for (int i = 0; i < 16; i++) {
        unsigned v;
        if (sscanf(h+2*i, "%2x", &v) != 1) return 0;
        s[i] = (byte)v;
    }
    return 1;
}

int main(int argc, char **argv) {
    byte s[16], key[16], w[176];
    if ((argc != 1 && argc != 3) ||
        !parse(argc == 3 ? argv[1] : "00112233445566778899AABBCCDDEEFF", s) ||
        !parse(argc == 3 ? argv[2] : "000102030405060708090A0B0C0D0E0F", key)) {
        puts("Usage: aes128 plaintext_hex key_hex");
        return 1;
    }
    print("Plaintext", s);
    print("Key", key);
    expand(key, w);
    puts("Round 0");
    print("Round key", w);
    add(s, w);
    print("AddRoundKey", s);
    for (int r = 1; r <= 10; r++) {
        printf("Round %d\n", r);
        print("Round key", w+16*r);
        for (int i = 0; i < 16; i++) s[i] = sub(s[i]);
        print("SubBytes", s);
        shift(s);
        print("ShiftRows", s);
        if (r != 10) {
            mix(s);
            print("MixColumns", s);
        }
        add(s, w+16*r);
        print("AddRoundKey", s);
    }
    print("Ciphertext", s);
    return 0;
}
