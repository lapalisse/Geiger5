//
//  DeltaBuffer.cpp
//  ArduinoTools
//
//  Created by Ludovic Bertsch on 18/06/2020.
//  Copyright © 2020 Ludovic Bertsch. All rights reserved.
//

#include <Arduino.h>
//#include <math.h>
#include <assert.h>

#include "DeltaBuffer.h"

template <class T>
DeltaBuffer<T>::DeltaBuffer(uint32_t size) {
    this->size = size;
    this->counts = new T[size + 1];

    // We initialize just one value:
    this->counts[size] = 0; 
    this->index = 0;

    // None read yet!
    this->n_read = 0;
}

template <class T>
uint32_t DeltaBuffer<T>::getIndex() {
    return index;
}

template <class T>
uint32_t DeltaBuffer<T>::getNRead() {
    return n_read;
}

template <class T>
bool DeltaBuffer<T>::hasSignificant(uint32_t n_values) {
  return this->n_read >= n_values;
}

template <class T>
void DeltaBuffer<T>::add(T count) {
    this->counts[this->index] = count;
    this->index = normalize(this->index + 1, this->size + 1);
    
    if (this->n_read != size) {
        this->n_read++;
    }
}

template <class T>
int32_t DeltaBuffer<T>::normalize(int32_t value, int32_t maxi) {
    while (value < 0) {
        value += maxi;
    }
    
    while (value >= maxi) {
        value -= maxi;
    }
    
    return value;
}

template <class T>
void DeltaBuffer<T>::reset(T count) {
    // We initialize just one value:
    this->counts[size] = count; 
    this->index = 0;

    // None read yet!
    this->n_read = 0;
}

template <class T>
T DeltaBuffer<T>::count_between(int32_t from, int32_t to) {
    assert(from <= to);
    assert(from <= 0);
    assert(to <= 0);
    
    uint32_t from_cell = normalize(this->index - 1 - min(-from, n_read), this->size + 1);
    uint32_t to_cell = normalize(this->index - 1 - min(-to, n_read), this->size + 1);
    
    return counts[to_cell] - counts[from_cell];
}

template <class T>
T DeltaBuffer<T>::count_last(uint32_t n_values) {
    /*uint32_t mini = min(n_values, this->n_read);

    return counts[normalize(this->index - 1, this->size + 1)]
        - counts[normalize(this->index - 1 - mini, this->size + 1)];*/
    return count_between(-n_values, 0);
}

template <class T>
double DeltaBuffer<T>::avg_last(uint32_t n_values) {
    uint32_t mini = min(n_values, this->n_read);

    if (mini == 0) {
      return 0.0; 
    } else {
      return double(count_last(mini)) / mini;
  }
}

template <class T>
DeltaBuffer<T>::~DeltaBuffer() {
    this->size = 0;
    delete[] this->counts;
    this->index = 0;
    this->n_read = 0;
}
