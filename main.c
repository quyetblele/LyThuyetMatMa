#include <stdio.h>
#include <stdint.h>

// Nhan hai da thuc trong GF(2^m), dung dich bit va XOR
unsigned int gf_mul(unsigned int a, unsigned int b, int m, unsigned int poly) {
    unsigned int result = 0;
    unsigned int mask = (1u << m) - 1u;
    unsigned int high = 1u << (m - 1);
    a &= mask;
    b &= mask;

    for (int i = 0; i < m; i++) {
        if (b & 1u)
            result ^= a;
        b >>= 1;
        int overflow = (a & high) != 0;
        a = (a << 1) & mask;
        if (overflow)
            a ^= poly & mask;
    }
    return result & mask;
}

int main(void) {
    unsigned int a, b;
    int m;

    printf("Chon truong (4 hoac 8): ");
    if (scanf("%d", &m) != 1 || (m != 4 && m != 8)) {
        printf("Truong khong hop le.\n");
        return 1;
    }

    printf("Nhap a va b (he 16, vi du 57 83): ");
    if (scanf("%x %x", &a, &b) != 2 ||
        a >= (1u << m) || b >= (1u << m)) {
        printf("Gia tri khong hop le.\n");
        return 1;
    }

    unsigned int poly = (m == 4) ? 0x13u : 0x11Bu;
    printf("Ket qua: 0x%X * 0x%X = 0x%X trong GF(2^%d)\n",
           a, b, gf_mul(a, b, m, poly), m);
    return 0;
}
