// 
// modem_soft_example.c
//
// This example demonstates soft demodulation of linear
// modulation schemes.
//

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include "liquid.h"

#define OUTPUT_FILENAME "modem_soft_example.m"

// print usage/help message
void usage()
{
    printf("modem_soft_example [options]\n");
    printf("  u/h   : print usage\n");
    printf("  p     : modulation depth (default 2 bits/symbol)\n");
    printf("  m     : modulation scheme (psk default)\n");
    liquid_print_modulation_schemes();
}

// print a string of bits to the standard output
//  _x      :   input symbol
//  _bps    :   bits/symbol
//  _n      :   number of characters to print (zero-padding)
void print_bitstring(unsigned int _x,
                     unsigned int _bps,
                     unsigned int _n)
{
    unsigned int i;
    for (i=0; i<_bps; i++)
        printf("%1u", (_x >> (_bps-i-1)) & 1);
    for (i=_bps; i<_n; i++)
        printf(" ");
}


int main(int argc, char*argv[])
{
    // create mod/demod objects
    unsigned int bps=2;
    modulation_scheme ms = LIQUID_MODEM_PSK;

    int dopt;
    while ((dopt = getopt(argc,argv,"uhm:p:")) != EOF) {
        switch (dopt) {
        case 'u':
        case 'h':
            usage();
            return 0;
        case 'm':
            ms = liquid_getopt_str2mod(optarg);
            if (ms == LIQUID_MODEM_UNKNOWN) {
                fprintf(stderr,"error: modem_example, unknown/unsupported modulation scheme \"%s\"\n", optarg);
                return 1;
            }
            break;
        case 'p':
            bps = atoi(optarg);
            break;
        default:
            fprintf(stderr,"error: modem_example, unknown option\n");
            usage();
            return 1;
        }
    }

    // create the modem objects
    modem mod   = modem_create(ms, bps);
    modem demod = modem_create(ms, bps);

    // ensure bits/symbol matches modem description (only
    // applicable to certain specific modems)
    bps = modem_get_bps(mod);

    modem_print(mod);

    unsigned int i;         // modulated symbol
    unsigned int s_hard;    // demodulated symbol (hard)
    unsigned char soft_bits[bps];
    unsigned int s_soft;    // demodulated symbol (soft, compacted)
    unsigned int num_symbols = 1<<bps;
    float complex x;
    unsigned int num_sym_errors = 0;
    unsigned int num_bit_errors = 0;

    printf("\n");
    printf("  %-11s %-11s %-11s  : ", "input sym.", "hard demod", "soft demod");
    for (i=0; i<bps; i++)
        printf("   b[%1u]", i);
    printf("\n");

    for (i=0; i<num_symbols; i++) {
        // modulate symbol
        modem_modulate(mod, i, &x);

        // demodulate, including soft decision
        modem_demodulate_soft(demod, x, &s_hard, soft_bits);

        // re-pack soft bits to hard decision
        liquid_pack_soft_bits(soft_bits, bps, &s_soft);

        // print results
        printf("  ");
        print_bitstring(i,     bps,12);
        print_bitstring(s_hard,bps,12);
        print_bitstring(s_soft,bps,12);
        printf(" : ");
        unsigned int j;
        for (j=0; j<bps; j++)
            printf("%7u", soft_bits[j]);
        printf("\n");

        num_sym_errors += i == s_soft ? 0 : 1;
        num_bit_errors += count_bit_errors(i,s_soft);
    }
    printf("num sym errors: %4u / %4u\n", num_sym_errors, num_symbols);
    printf("num bit errors: %4u / %4u\n", num_bit_errors, num_symbols*bps);

    modem_destroy(mod);
    modem_destroy(demod);
    return 0;
}