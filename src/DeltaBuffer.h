//
//  DeltaBuffer.h
//  ArduinoTools
//
//  Created by Ludovic Bertsch on 18/06/2020.
//  Copyright © 2020 Ludovic Bertsch. All rights reserved.
//

#ifndef DeltaBuffer_hpp
#define DeltaBuffer_hpp

#include <stdio.h>
#include <stdlib.h>

template <class T>
class DeltaBuffer {
protected:
    uint32_t size;
    uint32_t n_read;
    uint32_t index;
    T* counts;

    int32_t static normalize(int32_t value, int32_t maxi);
    
public:
    DeltaBuffer(uint32_t size);
    virtual ~DeltaBuffer();

    // debug only:
    uint32_t getIndex();
    uint32_t getNRead();
    
    bool hasSignificant(uint32_t n_values);
    void add(T count);
    void reset(T count = 0);

    T count_between(int32_t from, int32_t to); // from and to must be negative, 0 = now
    T count_last(uint32_t n_values); // n°values is positive
    double avg_last(uint32_t n_values); // idem
};

#endif /* DeltaBuffer_hpp */
