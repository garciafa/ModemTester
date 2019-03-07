//
// Created by garciafa on 05/03/19.
//

#ifndef MODEMTESTER_COMMUNICATINGRUNNABLEFACTORY_H
#define MODEMTESTER_COMMUNICATINGRUNNABLEFACTORY_H

#include "AISO.h"

template<typename RunnableType>
class CommunicatingRunnableFactory {
public:

    explicit CommunicatingRunnableFactory(AISOBase *ioStream);

    virtual ~CommunicatingRunnableFactory() = default;

    virtual RunnableType* operator()();
protected:
    AISOBase* _ioStream;
};

template<typename RunnableType>
CommunicatingRunnableFactory<RunnableType>::CommunicatingRunnableFactory (AISOBase *ioStream) : _ioStream(ioStream){}

template<typename RunnableType>
RunnableType *CommunicatingRunnableFactory<RunnableType>::operator() ()
{
    return new RunnableType(_ioStream);
}

#endif //MODEMTESTER_COMMUNICATINGRUNNABLEFACTORY_H
