//
// Created by Wenxin Zheng on 2021/4/21.
//
#ifndef ACMOS_SPR21_ANSWER_PRINTK_H
#define ACMOS_SPR21_ANSWER_PRINTK_H

static void printk_write_string(const char *str) {
    // Homework 1: YOUR CODE HERE
    // this function print string by the const char pointer
    // I think 3 lines of codes are enough, do you think so?
    // It's easy for you, right?
    for(int i=0;str[i]!='\0';++i) {
        uart_putc(str[i]);
    }
}


static void printk_write_num(int base, unsigned long long n, int neg) {
    // Homework 1: YOUR CODE HERE
    // this function print number `n` in the base of `base` (base > 1)
    // you may need to call `printk_write_string`
    // you do not need to print prefix like "0x", "0"...
    // Remember the most significant digit is printed first.
    // It's easy for you, right?
    int a[100];
    int ln=0;
    while(n){
        a[++ln]=n%base;
        n/=base;
    }
    char out[100];
    int i=0;
    if(neg)out[i++]='-';
    while(ln){
        if(a[ln]>9)out[i]=a[ln]-10+'a';
        else out[i]=a[ln]+'0';
        --ln;++i;
    }
    out[i]='\0';
    printk_write_string(out);
}

#endif  // ACMOS_SPR21_ANSWER_PRINTK_H
