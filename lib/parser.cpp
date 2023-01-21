#include <iterator>
#include <fstream>
#include "parser.h"

bool omfl::OmflData::valid() const {
  return valid_flag;
}
bool omfl::OmflData::KeyValid(const std::string& key) {
  for (char c : key) {
    if (!((bool) std::isalnum(c) or c == '-' or c == '_')) {
      return false;
    }
  }
  if (key.empty()) {
    return false;
  }

  return true;
}
bool omfl::OmflData::ValueValid(const std::string& value, char& type_value) {
  if (value.empty() and type_value != 'v') {
    return false;
  }
  if (type_value == 'b') {
    if (!(value == "true" or value == "false")) {
      return false;
    }
  } else if (type_value == 'i') {
    for (int i = 1; i < value.size(); i++) {
      if (!(std::isdigit(value[i]))) {
        return false;
      }
    }
    if (value[0] == '+' or value[0] == '-') {
      if (value.size() == 1) {
        return false;
      }
    } else if (!(std::isdigit(value[0]))) {
      return false;
    }
  } else if (type_value == 'f') {
    if (value[0] == '.' or value[value.size() - 1] == '.') {
      return false;
    }
    for (int i = 1; i < value.size(); i++) {
      if (!(std::isdigit(value[i]) or value[i] == '.')) {
        return false;
      }
    }
    if (value[0] == '+' or value[0] == '-') {
      if (value.size() == 1) {
        return false;
      }
      if (value[1] == '.') {
        return false;
      }
    } else if (!(std::isdigit(value[0]))) {
      return false;
    }
    for (int i = 1; i < value.size() - 1; i++) {
      if (value[i] == '.') {
        if (!(std::isdigit(value[i + 1])) and !(std::isdigit(value[i + 1]))) {
          return false;
        }
      }
    }
  } else if (type_value == 's') {
    if (!(value[0] == '\"' and value[value.size() - 1] == '\"')) {
      return false;
    }
  } else if (type_value == 'v') {
    bool string_flag = false;
    int cnt_open;
    cnt_open = 0;
    int cnt_close;
    cnt_close = 0;
    for (char c : value) {
      if (c == '\"') {
        string_flag = !string_flag;
      }
      if (!string_flag) {
        if (c == '[') {
          cnt_open++;
        } else if (c == ']') {
          cnt_close++;
        }
      }
      if (cnt_close > cnt_open) {
        return false;
      }
    }
    if (cnt_open != cnt_close) {
      return false;
    }
  } else {
    return false;
  }

  return true;
}
bool omfl::OmflData::SectionsValid(const std::string& sections) {
  bool flag = true;
  if (sections.size() <= 2) {
    flag = false;
  } else {
    if (sections[0] != '[' or sections[sections.size() - 1] != ']' or sections[1] == '.'
        or sections[sections.size() - 2] == '.') {
      flag = false;
    }
  }
  char prev_c;
  prev_c = '.';
  for (char section : sections) {
    if (prev_c == '.' and section == '.') {
      flag = false;
    }
    prev_c = section;
  }

  return flag;
}

std::vector<std::string> omfl::OmflData::SplitSections(const std::string& sections) {
  std::vector<std::string> vec_sections;
  vec_sections.emplace_back("");
  for (char c : sections) {
    if (c == '.') {
      vec_sections.emplace_back("");
    } else if (c != '[' and c != ']') {
      vec_sections[vec_sections.size() - 1].push_back(c);
    }
  }

  return vec_sections;
}

omfl::ElementData* omfl::OmflData::AppendSection(const std::string& key, omfl::ElementData* cur_section) {
  omfl::ElementData new_element_data;
  std::map<std::string, ElementData> new_map;
  new_element_data.value = new_map;
  std::get<map_ed>(cur_section->value).insert({key, new_element_data});

  return &(std::get<map_ed>(cur_section->value).find(key)->second);
}

std::vector<omfl::ElementData> omfl::OmflData::NewVectorElement(std::string& strvec) {
  std::vector<omfl::ElementData> this_vec;
  char type_value = 'n';
  std::string cur_value;
  strvec.push_back(',');
  std::stringstream vecstream(strvec);
  bool flag_string = false;
  bool flag_vector = false;
  bool flag_vector_start = true;
  uint16_t open_v = 0;
  uint16_t close_v = 0;
  char chr;
  while (!vecstream.eof()) {
    chr = (char) vecstream.get();
    if (type_value == 'n') {
      if (chr == '[') {
        type_value = 'v';
        flag_vector = true;
      } else if (chr == '\"') {
        type_value = 's';
      } else if (std::isdigit(chr) or chr == '+' or chr == '-') {
        type_value = 'i';
      } else if (chr == 't' or chr == 'f') {
        type_value = 'b';
      }
    }
    if (type_value == 'b') {
      if (chr != ',') {
        cur_value.push_back(chr);
      } else {
        omfl::ElementData new_element_data;
        if (!ValueValid(cur_value, type_value)) {
          valid_flag = false;
        }
        if (cur_value == "true") {
          new_element_data.value = true;
        } else {
          new_element_data.value = false;
        }
        type_value = 'n';
        cur_value = "";
        this_vec.push_back(new_element_data);
      }
    } else if (type_value == 'i' or type_value == 'f') {
      if (chr != ',') {
        cur_value.push_back(chr);
        if (chr == '.') {
          type_value = 'f';
        }
      } else {
        omfl::ElementData new_element_data;
        if (!ValueValid(cur_value, type_value)) {
          valid_flag = false;
        }
        if (type_value == 'i') {
          new_element_data.value = std::stoi(cur_value);
        } else {
          new_element_data.value = std::stof(cur_value);
        }
        type_value = 'n';
        cur_value = "";
        this_vec.push_back(new_element_data);
      }
    } else if (type_value == 's') {
      if (chr == '\"') {
        flag_string = !flag_string;
      }
      if (flag_string or chr == '\"') {
        cur_value.push_back(chr);
      } else {
        omfl::ElementData new_element_data;
        new_element_data.value = cur_value;
        cur_value = "";
        type_value = 'n';
        this_vec.push_back(new_element_data);
        flag_string = false;
      }
    } else if (type_value == 'v') {
      if (chr == '\"') {
        flag_string = !flag_string;
      }
      if (!flag_string) {
        if (chr == '[') {
          open_v++;
        } else if (chr == ']') {
          close_v++;
        }
      }
      if (open_v == close_v and !flag_vector_start) {
        flag_vector = false;
      }
      if (!flag_vector_start and flag_vector) {
        cur_value.push_back(chr);
      }
      flag_vector_start = false;
      if (!flag_vector and chr == ',') {
        omfl::ElementData new_element_data;
        new_element_data.value = NewVectorElement(cur_value);
        type_value = 'n';
        cur_value = "";
        this_vec.push_back(new_element_data);
        flag_string = false;
        flag_vector = false;
        flag_vector_start = true;
        open_v = 0;
        close_v = 0;
      }
    }
  }

  return this_vec;
}

void omfl::OmflData::AppendSimpleElement(const std::string& key,
                                         std::string& value,
                                         char type_element,
                                         omfl::ElementData* cur_section) {
  omfl::ElementData new_element_data;
  if (type_element == 'b') {
    if (value == "true") {
      new_element_data.value = true;
    } else {
      new_element_data.value = false;
    }
  } else if (type_element == 'i') {
    new_element_data.value = std::stoi(value);
  } else if (type_element == 'f') {
    new_element_data.value = std::stof(value);
  } else if (type_element == 's') {
    new_element_data.value = value;
  } else if (type_element == 'v') {
    new_element_data.value = OmflData::NewVectorElement(value);
  }
  if (std::get<map_ed>(cur_section->value).count(key) == 1) {
    valid_flag = false;
  }
  std::get<map_ed>(cur_section->value).insert({key, new_element_data});
}

omfl::OmflData omfl::uniparse(std::iostream& datastream) {
  omfl::OmflData myOmflData;
  std::string word;
  std::vector<std::string> cur_sections;
  ElementData* cur_section;
  cur_section = myOmflData.global_data;
  while (datastream >> word) {
    if (word[0] == '[') {
      if (!myOmflData.SectionsValid(word)) {
        myOmflData.valid_flag = false;

        return myOmflData;
      }
      cur_sections = myOmflData.SplitSections(word);
      cur_section = myOmflData.global_data;
      for (std::string& i : cur_sections) {
        if (std::get<map_ed>(cur_section->value).count(i)) {
          cur_section = &((std::get<map_ed>(cur_section->value)).find(i))->second;
        } else {
          cur_section = myOmflData.AppendSection(i, cur_section);
        }
      }
    } else if (word[0] == '#') {
      if (word[word.size() - 1] != '\n') {
        char c;
        c = (char) datastream.get();
        while (c != '\n' and c != '\377') {
          c = (char) datastream.get();
        }
      }
    } else {
      datastream.seekg(-((int) word.size()), std::ios::cur);
      char chr;
      chr = (char) datastream.get();
      std::string key;
      while (chr != '=' and chr != '\377') {
        if (chr == '\n') {
          myOmflData.valid_flag = false;
          return myOmflData;
        } else if (chr != ' ') {
          key.push_back(chr);
        }
        chr = (char) datastream.get();
      }
      if (!myOmflData.KeyValid(key)) {
        myOmflData.valid_flag = false;

        return myOmflData;
      }
      std::string value;
      char type_value = 'n'; //null
      bool flag_string_start = true;
      bool flag_vector_start = true;
      uint16_t open_v = 0;
      uint16_t close_v = 0;
      while (true) {
        chr = (char) datastream.get();
        if (chr == '\n' and flag_string_start) {
          if (type_value == 'n' or type_value == 'v') {
            myOmflData.valid_flag = false;

            return myOmflData;
          }
        }
        if (type_value == 'n') {
          if (chr == '[') {
            type_value = 'v';
          } else if (chr == '\"') {
            type_value = 's';
          } else if (std::isdigit(chr) or chr == '+' or chr == '-') {
            type_value = 'i';
          } else if (chr != ' ') {
            type_value = 'b';
          }
        }
        if (type_value == 'b') {
          if (chr == ' ' or chr == '\n' or chr == '\377') {
            break;
          }
          value.push_back(chr);
        } else if (type_value == 'i' or type_value == 'f') {
          if (chr == ' ' or chr == '\n' or chr == '\377') {
            break;
          }
          if (chr == '.') {
            type_value = 'f';
          }
          value.push_back(chr);
        } else if (type_value == 's') {
          value.push_back(chr);
          if (chr == '\"' and !flag_string_start or chr == '\377') {
            break;
          }
          flag_string_start = false;
        } else if (type_value == 'v') {
          if (chr == '\"') {
            flag_string_start = !flag_string_start;
          }
          if (flag_string_start) {
            if (chr == '[') {
              open_v++;
            } else if (chr == ']') {
              close_v++;
            }
          }
          if (open_v == close_v and !flag_vector_start or chr == '\377') {
            if (open_v != close_v) {
              myOmflData.valid_flag = false;
            }
            break;
          }
          if (!flag_vector_start and !(flag_string_start and chr == ' ')) {
            value.push_back(chr);
          }
          flag_vector_start = false;
        }
      }
      if (!myOmflData.ValueValid(value, type_value)) {
        myOmflData.valid_flag = false;

        return myOmflData;
      }
      myOmflData.AppendSimpleElement(key, value, type_value, cur_section);
    }
  }

  return myOmflData;
}

omfl::OmflData omfl::parse(const std::string& str) {
  std::stringstream datastream(str);
  return uniparse(datastream);
}

omfl::OmflData omfl::parse(const std::filesystem::path& path) {
  std::fstream datastream;
  datastream.open(path);
  return uniparse(datastream);
}

bool omfl::ElementData::IsInt() const {
  if (std::holds_alternative<int>(this->value)) {
    return true;
  }

  return false;
}
int omfl::ElementData::AsInt() {
  return std::get<int>(this->value);
}
int omfl::ElementData::AsIntOrDefault(int def) {
  if (std::holds_alternative<int>(this->value)) {
    return std::get<int>(this->value);
  }

  return def;
}
bool omfl::ElementData::IsFloat() const {
  if (std::holds_alternative<float>(this->value)) {
    return true;
  }

  return false;
}
float omfl::ElementData::AsFloat() {
  return std::get<float>(this->value);
}
float omfl::ElementData::AsFloatOrDefault(float def) {
  if (std::holds_alternative<float>(this->value)) {
    return std::get<float>(this->value);
  }

  return def;
}
bool omfl::ElementData::IsString() const {
  if (std::holds_alternative<std::string>(this->value)) {
    return true;
  }

  return false;
}
std::string omfl::ElementData::AsString() {
  std::string str;
  str = (std::get<std::string>(this->value));

  return str.substr(1, str.size() - 2);
}
std::string omfl::ElementData::AsStringOrDefault(std::string def) {
  if (std::holds_alternative<std::string>(this->value)) {
    return std::get<std::string>(this->value);
  }

  return def;
}
bool omfl::ElementData::IsArray() const {
  if (std::holds_alternative<std::vector<ElementData>>(this->value)) {
    return true;
  }

  return false;
}
bool omfl::ElementData::IsBool() const {
  if (std::holds_alternative<bool>(this->value)) {
    return true;
  }

  return false;
}
bool omfl::ElementData::AsBool() {
  return std::get<bool>(this->value);
}

omfl::ElementData& omfl::OmflData::UniGet(const std::string& key, omfl::ElementData* this_element) {
  std::vector<std::string> sections;
  sections = SplitSections(key);
  for (auto& section : sections) {
    std::map<std::basic_string<char>, omfl::ElementData>::iterator check_element;
    check_element = (std::get<map_ed>(this_element->value)).find(section);
    if ((std::get<map_ed>(this_element->value)).end() == check_element) {
      omfl::EmptyElement em;

      return *(em.value);
    }
    this_element = &(check_element->second);
  }

  return *this_element;
}
omfl::ElementData& omfl::OmflData::Get(const std::string& key) const {
  return UniGet(key, global_data);
}
omfl::ElementData& omfl::ElementData::Get(const std::string& key) {
  return OmflData::UniGet(key, this);
}
omfl::ElementData& omfl::ElementData::operator[](const int i) {
  if ((std::get<vector_ed>(this->value)).size() < i + 1) {
    omfl::EmptyElement em;
    return *(em.value);
  }

  return (std::get<vector_ed>(this->value))[i];
}


