#pragma once

#include <filesystem>
#include <istream>
#include <map>
#include <variant>
#include <sstream>
#include <optional>

namespace omfl {

struct ElementData;

typedef std::vector<ElementData> vector_ed;
typedef std::map<std::string, ElementData> map_ed;
typedef std::variant<typename std::vector<ElementData>,   //v
                     std::map<std::string, ElementData>,  //m
                     int,                                 //i
                     float,                               //f
                     std::string,                         //s
                     bool> AllTypes;                      //b

struct ElementData {
  AllTypes value;

  int AsInt();
  bool AsBool();
  float AsFloat();
  std::string AsString();
  int AsIntOrDefault(int def);
  float AsFloatOrDefault(float def);
  std::string AsStringOrDefault(std::string def);
  [[nodiscard]] bool IsInt() const;
  [[nodiscard]] bool IsFloat() const;
  [[nodiscard]] bool IsString() const;
  [[nodiscard]] bool IsArray() const;
  [[nodiscard]] bool IsBool() const;

  [[nodiscard]] ElementData& Get(const std::string& key);
  [[nodiscard]] ElementData& operator[](const int i);
};
struct EmptyElement {
  ElementData* value = new ElementData;
  ~EmptyElement() {
    delete value;
  };
};

class OmflData {
 public:
  bool valid_flag;
  ElementData* global_data;
  explicit OmflData() {
    global_data = new ElementData;
    valid_flag = true;
    std::map<std::string, ElementData> new_map;
    global_data->value = new_map;
  }
  ~OmflData() {
    delete global_data;
  }
  [[nodiscard]] ElementData& Get(const std::string& key) const;
  static ElementData& UniGet(const std::string& key, omfl::ElementData* this_element);
  [[nodiscard]] bool valid() const;

  static std::vector<std::string> SplitSections(const std::string& sections);
  static omfl::ElementData* AppendSection(const std::string& key, omfl::ElementData* cur_section);
  void AppendSimpleElement(const std::string& key,
                           std::string& value,
                           char type_element,
                           omfl::ElementData* cur_section);
  std::vector<omfl::ElementData> NewVectorElement(std::string& strvec);

  static bool KeyValid(const std::string& key);
  static bool ValueValid(const std::string& value, char& type_value);
  static bool SectionsValid(const std::string& sections);
};
OmflData parse(const std::filesystem::path& path);
OmflData parse(const std::string& str);
OmflData uniparse(std::iostream& datastream);

}// namespace