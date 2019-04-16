//
// Created by pbx on 15/04/19.
//

#include "printf.h"
#include "meloader.h"
void *get_tls_ptr(int i) {
    int t =  get_current_thread_id();
    void *tls = get_thread_tls(t);
    return tls - 4 * i;
}