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

#ifndef H_DC_SETTINGS_INCLUDED
#define H_DC_SETTINGS_INCLUDED

#include <string>
#include <vector>
namespace Dynacoe {

class SettingsData;

/// \brief Stores dynamically configured settings.
///
///  Instead of cluttering interface space by having Set/Getters of basic 
///  values that are rarely used, you can offload these proprties to a 
///  settings object that can handle all properties internally with a 
///  runtime-oriented interface. It is not always appropriate to utilize the 
///  Settings object to replace your setters/getters; however, in cases 
///  where setting are not common set externally, it can be a good alternative.
class Settings {
  public: 
    Settings();
    ~Settings();

    using GetCallback = std::string (*)(const std::string & name);
    using SetCallback = void        (*)(const std::string & name, const Chain & data);


    /// \brief Adds a new setting by name. If info is given, it will be used in 
    /// the summary. defaultValue is given to the property and does call the setter.
    ///
    void Add(const std::string & name, GetCallback, SetCallback, const std::string & info = "");


    /// \brief Adds a new settings by name, but that setting is only externally settable.
    ///
    void AddReadOnly(const std::string & name, GetCallback, const std::string & info);


    /// \brief Sets the value of a setting. If this setting is read-only, 
    /// no action is taken.
    ///
    void Set(const std::string & name, const Chain & );
    

    /// \brief Gets the value for the setting given.
    ///
    /// If the settings doesn't exist or is otherwise unavailable, an empty string is returned.
    Chain Get(const std::string & name) const;

    /// \brief Gets the info for the setting given.
    ///
    /// If the settings doesn't exist or is otherwise unavailable, an empty string is returned.
    std::string GetInfo(const std::string & name) const;


    /// \brief Returns a list of all the known settings.
    ///
    const std::vector<std::string> & List() const;

    /// \brief Returns a list of all the known settings for this instance
    /// including their current values, whether its read-only, and any info.
    ///
    /// The string is formatted for easy viewing.
    std::string GetSummary() const;

  private:
    SettingsData * X;

};


}

#endif
