
/*

Copyright (c) 2018, Johnathan Corkery. (jcorkery@umich.edu)
All rights reserved.

This file is part of the Dynacoe project (https://github.com/jcorks/Dynacoe)
Dynacoe was released under the MIT License, as detailed below.



Permission is hereby granted, free of charge, to any person obtaining a copy 
of this software and associated documentation files (the "Software"), to deal 
in the Software without restriction, including without limitation the rights 
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
copies of the Software, and to permit persons to whom the Software is furnished 
to do so, subject to the following conditions:

The above copyright notice and this permission notice shall
be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
DEALINGS IN THE SOFTWARE.



*/
#ifndef DC_REFBANK_H_INCLUDED
#define DC_REFBANK_H_INCLUDED



#include <set>
#include <map>


namespace Dynacoe {
    




template<typename T>
/** \brief A simple, generic reference counter
 *
 * Refbank represents a generic, multi-instance reference counter.
 * RefBank manages a number of accounts. Each account 
 * signifies a set of references. Accounts are uniquely identified
 * by the user supplied object T and are automatically created once Deposit() is called. 
 * Each account has a reference balance associated with it. Once an
 * account balance has reached 0 or less, the account is removed. If an AccountRemover is set
 * , the AccountRemover callback will be run on the account before removal.


 */
class RefBank {
  public:

    /** \brief Class to handle the case where all references
     * to an object are exhausted.
     */
    class AccountRemover {
      public:
        virtual void operator()(const T &) = 0;
    };

    RefBank();
    
    /// \brief Adds a reference count amount.
    /// 
    /// If a reference count deposite call would yeild a deposit greater than
    /// UINT32_MAX, the account is set to UINT32_MAX. (UINT32_MAX is defined in cstdint.h)
    /// @param account The object representing the reference to add counts to
    /// @param amount The reference count amount to add. The default is one.
    void Deposit(const T & account, uint32_t amount = 1);

    /// \brief Deduct a reference count amount.
    ///
    /// If a reference withdrawl would result in a balance of 0 or lower, the 
    /// AccountHandler is called with the account and is removed.
    /// @param account The object representing the reference to remove counts from.
    /// @param amount The reference count amount to deduct. The default is one.
    void Withdraw(const T & account, uint32_t amount = 1);

    /// \brief Returns the number of reference counts accumulated in the account.
    ///
    /// If the count does not exist, 0 is returned.
    uint32_t GetBalance(const T & account);


    /// \brief Removes all accounts and runs the AccountRemover on all
    /// accounts.
    /// 
    void WithdrawAll();

    /// \brief Sets the account remover callback.
    ///
    /// The lifetime of the source object should match the lifetime of this RefBank
    void SetAccountRemover(AccountRemover *);

  private:
    std::map<T, uint32_t> counter;
    AccountRemover * collector;

};











template<typename T>
RefBank<T>::RefBank() {
    collector = nullptr;
}



template<typename T>
void RefBank<T>::Deposit(const T & ref, uint32_t amount) {
    uint32_t oldBalance = counter[ref];
    counter[ref] = (oldBalance + amount < oldBalance ? UINT32_MAX : oldBalance + amount);
    
}

template<typename T>
void RefBank<T>::Withdraw(const T & ref, uint32_t amount) {
    auto iter = counter.find(ref);
    if (iter != counter.end()) {
        iter->second -= (iter->second < amount ? iter->second : amount);
        if (iter->second == 0) {
            if (collector)(*collector)(ref);
            counter.erase(ref);
        }
    }
} 

template<typename T> 
uint32_t RefBank<T>::GetBalance(const T & ref) {
    return counter[ref];
}

template<typename T>
void RefBank<T>::WithdrawAll() {
    for(const std::pair<T, uint32_t> & ref : counter) {
        if (collector)
            (*collector)(ref.first);
    }
    counter.clear();
}


template<typename T>
void RefBank<T>::SetAccountRemover(RefBank<T>::AccountRemover * col) {
    collector = col;
}

}


#endif
