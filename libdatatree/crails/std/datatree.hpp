#ifndef  STD_DATATREE_HPP
# define STD_DATATREE_HPP

# include <boost/property_tree/ptree.hpp>
# include <functional>
# include <vector>
# include <map>
# include <string>
# include <string_view>
# include <iostream>
# include <crails/utils/backtrace.hpp>

class DataTree;

class Data
{
  friend class DataTree;

  void overload_path(const std::string& path) { this->path = path; }
protected:
  Data(boost::property_tree::ptree& tree, const std::string_view key) :
    tree(&tree),
    context(""),
    key(key),
    path(key)
  {
  }

  Data(boost::property_tree::ptree& tree, const std::string& context, const std::string_view key) :
    tree(&tree),
    context(context),
    key(key),
    path(context.size() > 0 ? (context + '.' + key.data()) : key)
  {
  }

public:
  Data operator[](const std::string_view key) const
  {
    if (key.length() == 0)
      throw boost_ext::invalid_argument("Data::operator[] cannot take an empty string");
    return Data(*tree, path, key);
  }

  Data operator[](const char* key) const
  {
    return operator[](std::string_view(key));
  }

  Data at(unsigned int i) const;

  std::vector<std::string> find_missing_keys(const std::vector<std::string>& keys) const;
  bool                     require(const std::vector<std::string>& keys) const;

  const std::string& get_path() const { return path; }
  const std::string& get_key()  const { return key; }

  std::size_t count() const
  {
    return tree->get_child(path).size();
  }

  template<typename T>
  T as() const
  {
    try { return tree->get<T>(path); }
    catch (std::exception& e) { throw boost_ext::runtime_error(e.what()); }
  }

  template<typename T>
  T defaults_to(const T def) const { return tree->get(path, def); }

  template<typename T>
  operator T() const
  {
    try { return tree->get<T>(path); }
    catch (std::exception& e) { throw boost_ext::runtime_error(e.what()); }
  }

  template<typename T>
  std::vector<T> to_vector() const
  {
    std::vector<T> array;
    auto& tree = (path == "") ? *(this->tree) : this->tree->get_child(path);

    for (boost::property_tree::ptree::value_type& v : tree)
      array.push_back(v.second.get<T>(v.first));
    return array;
  }

  template<typename T>
  void from_vector(const std::vector<T>& array)
  {
    auto& tree_array = require_ptree();

    for (const T& v : array)
    {
      boost::property_tree::ptree child;

      child.put("", v);
      tree_array.push_back(std::make_pair("", child));
    }
  }

  template<typename T>
  operator std::vector<T>() const
  {
    return to_vector<T>();
  }

  template<typename T>
  Data& operator=(const T value)
  {
    tree->put(path, value);
    return *this;
  }

  template<typename T>
  Data& operator=(const std::vector<T>& value)
  {
    from_vector(value);
    return *this;
  }

  Data& operator=(const Data& copy)
  {
    tree    = copy.tree;
    key     = copy.key;
    context = copy.context;
    path    = copy.path;
    return *this;
  }

  template<typename T>
  bool operator==(const T value) const
  {
    return tree->get<T>(path) == value;
  }

  template<typename T>
  bool operator!=(const T value) const { return !(Data::operator==(value)); }

  Data operator||(Data value) const
  {
    return exists() ? *this : value;
  }
  
  template<typename T>
  T operator||(const T value) const
  {
    return defaults_to<T>(value);
  }

  void push_back(Data data)
  {
    if (!(exists()))
    {
      boost::property_tree::ptree array;

      array.push_back(std::make_pair("", data.get_ptree()));
      tree->add_child(path, array);
    }
    else
      get_ptree().push_back(std::make_pair("", data.get_ptree()));
  }

  template<typename T>
  void push_back(const T value)
  {
    boost::property_tree::ptree child;

    child.put("", value);
    push_back(Data(child, ""));
  }

  bool is_null() const;
  bool is_blank() const;
  bool is_array() const;
  bool exists() const;
  void destroy();

  void each(std::function<bool (Data)> functor);
  void each(std::function<bool (const Data)> functor) const;

  void output(std::ostream& out = std::cout) const;
  std::string to_json() const;
  std::string to_xml() const;

  void merge(Data data);
  void merge(DataTree data_tree);

  boost::property_tree::ptree& require_ptree();
  boost::property_tree::ptree& get_ptree() { return tree->get_child(path); }
  const boost::property_tree::ptree& get_ptree() const { return tree->get_child(path); }

  std::vector<std::string> get_keys() const;

  class iterator
  {
    friend class Data;
  public:
    using iterator_type = std::input_iterator_tag;
    using value_type = Data;

    iterator(const iterator& copy) : internal_iterator(copy.internal_iterator) {}
    iterator(boost::property_tree::ptree::iterator it) : internal_iterator(it) {}
  
    operator Data() const
    {
      Data data(internal_iterator->second, internal_iterator->first);
      data.overload_path("");
      return data;
    }

    Data operator*() { return Data(*this); }
    iterator& operator++() { internal_iterator++; return *this; }
    bool operator==(const iterator& other) const { return other.internal_iterator == internal_iterator; }
    bool operator!=(const iterator& other) const { return !operator==(other); }

  private:
    boost::property_tree::ptree::iterator internal_iterator;
  };

  iterator begin() const;
  iterator end() const;
  iterator erase(iterator deleted);

private:
  boost::property_tree::ptree* tree;
  std::string                  context, key, path;
};
 
template<> std::wstring Data::as<std::wstring>() const;

class DataTree
{
public:
  operator   Data()                                       { return as_data();       }
  Data       as_data()                                    { return Data(tree, "");  }
  const Data as_data() const                              { return Data(tree, "");  }
  Data       operator[](const std::string_view key)       { return Data(tree, key); }
  const Data operator[](const std::string_view key) const { return Data(tree, key); }
  void       clear()                                      { tree.clear(); }

  DataTree& from_map(const std::map<std::string, std::string>&);

  DataTree& from_json(std::stringstream&);
  DataTree& from_json(const std::string&);
  DataTree& from_json_file(const std::string&);
  std::string to_json() const;

  DataTree& from_xml(std::stringstream&);
  DataTree& from_xml(const std::string&);
  DataTree& from_xml_file(const std::string&);
  std::string to_xml() const;

  Data::iterator begin() const { return as_data().begin(); }
  Data::iterator end() const { return as_data().end(); }
  Data::iterator erase(Data::iterator it) { return as_data().erase(it); }

  boost::property_tree::ptree&       get_ptree()       { return tree; }
  const boost::property_tree::ptree& get_ptree() const { return tree; }

private:
  mutable boost::property_tree::ptree tree;
};

#endif
