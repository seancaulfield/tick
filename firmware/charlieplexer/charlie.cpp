/*
 * charlie.cpp
 * 
 * Library code to handle charlieplexing
 *
 * Author: Sean Caulfield <sean@yak.net>
 * License: GPLv2
 *
 */

#include "charlie.h"

CharlieDigit::CharlieDigit(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f, uint8_t g, uint8_t p, uint8_t k) {
  this->a = a;
  this->b = b;
  this->c = c;
  this->d = d;
  this->e = e;
  this->f = f;
  this->g = g;
  this->p = p;
  this->k = k;
}

void CharlieDigit::off() {
}

void CharlieDigit::write(uint8_t c) {
}
