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

#ifndef H_CHAIN_INCLUDED
#define H_CHAIN_INCLUDED

#include <Dynacoe/Util/Vector.h>
#include <string>
namespace Dynacoe {



/**
 *
 * \brief Allows for easy string building and deconstructing
 * 
 * Chains have 2 primary responsibilities:
 * - Allow a mechanism to easily concatenate multiple different types
 * - Break up a string into consumable tokens.
 *
 *
 */
class Chain {

  public:

    

    Chain();
        
    /// \name Instantiation
    ///
    /// A chain can be initalized from any of the following types:
    ///\{
    Chain(const char *);
    Chain(char);
    Chain(int);
    Chain(uint32_t);
    Chain(uint64_t);
    Chain(double);
    Chain(const Vector &);
    Chain(const Chain &);
    Chain(const std::string & other);
    Chain(const std::vector<Chain> &); 
    ///\}

    ~Chain();

    
    /// \name Numerical conversion to other types
    ///
    /// If the type has no correct conversion, a default 
    /// value of 0 is used.
    ///\{
    int AsInt();
    uint32_t AsUInt32();
    uint64_t AsUInt64();
    double AsDouble();
    ///\}


    /// \break Appends *this with an input
    ///
    Chain & operator<<(const Chain &);



    /// \brief Returns a std::string equivalent of this Chain.
    ///
    const std::string & ToString() const {
        return data;
    }
    

    /// \brief Assign a value to be the Chain
    ///
    Chain & operator=(const Chain &);



    /// \brief copy constructor allowing for implicit conversion to std::string s.
    ///
    operator std::string() const {
        return data;
    }


    /// \name Translate the Chain to and from the Al Bhed language.
    ///
    ///\{
    std::string TranslateToAlBhed() const;
    std::string TranslateFromAlBhed() const;
    ///\}


    /// \name Goes to the next token in the Chain.
    /// If there are not more links in the chain,
    /// the current link is an empty Chain.
    ///\{
    void NextLink();
    Chain & operator++(int);
    ///\}

    /// \brief returns the current link;
    ///
    std::string GetLink();

    /// \brief Returns the position in the whole string where the current link begins
    ///
    uint32_t GetLinkPos();



    /// \brief returns if there are any links left
    ///
    bool LinksLeft();

    /// \brief Goes back to the first link for reading.
    ///
    void GoToFirst();


    /// \brief Set the delimiters to define what each link is composed of.
    /// The default delimiters are space,
    /// newline, tab, and return.
    ///
    /// @param chars A string of characters where each character is a delimiter.
    void SetDelimiters(const std::string & chars);





  private:
    std::string data;
    std::string delimiters;
    
    uint32_t bufferPos;
    uint32_t links;
    std::string * cLink;

    bool curCharGood();
};
}

#endif
