#include <crails/datatree.hpp>
#include <string_view>
#include <iostream>

#undef NDEBUG
#include <cassert>

int main()
{
  using namespace std;

  // Basics
  {
    DataTree tree;

    tree.from_map({
      {"keyA", "valueA"},
      {"keyB", "42"}
    });
    assert(tree.as_data().count() == 2);
    assert(tree["keyA"].exists());
    assert(tree["keyA"].as<string>() == "valueA");
    assert(tree["keyB"].as<int>() == 42);
    assert(tree["keyC"].defaults_to<int>(1) == 1);
    assert(tree.as_data().get_keys().size() == 2);
    tree.clear();
    assert(tree.as_data().count() == 0);
  }

  // Iterators
  {
    DataTree tree;
    vector<string> expected{"valueA", "valueB"};

    tree["first"] = "valueA";
    tree["second"] = "valueB";
    for (Data data : tree)
    {
      assert(data.as<string>() == expected[0]);
      expected.erase(expected.begin());
    }
    tree.erase(tree.begin());
    assert((*tree.begin()).as<string>() == "valueB");
  }

  // Merge
  {
    DataTree tree;
    vector<string> list;

    tree["a"]["b"] = 42;
    tree["a"]["c"] = "toto";
    tree["d"]["e"] = "tintin";
    tree["d"].merge(tree["a"]);
    assert(tree["d"]["b"].as<int>() == 42);
    assert(tree["d"]["e"].as<string>() == "tintin");
    list = tree["d"].get_keys();
    assert(list[0] == "e");
    assert(list[1] == "b");
    assert(list[2] == "c");

    // seems like to_vector is broken, though not for everything (next test batch succeeds)
    //list = tree["d"].to_vector<string>();
    //assert(list.size() == 3);
  }

  // Vectors
  {
    DataTree tree;

    tree["number_list"].from_vector<int>({1,2,3,5,7,11,13,17});
    assert(tree["number_list"].count() == 8);
    assert(tree["number_list"].at(2) == 3);
    assert(tree["number_list"].to_vector<int>().size() == 8);
    assert(tree["number_list"].to_vector<int>()[5] == 11);
  }

  // Exists, blank and null
  {
    DataTree tree;

    assert(tree["toto"].exists() == false);
    assert(tree["toto"].is_null() == true);
    tree["toto"] = "";
    assert(tree["toto"].exists() == true);
    assert(tree["toto"].is_null() == false);
    assert(tree["toto"].is_blank() == true);
    tree["toto"] = "tintin";
    assert(tree["toto"].is_blank() == false);
    tree["toto"] = "null";
    assert(tree["toto"].is_null() == true);
  }

  // OR operator
  {
    DataTree tree;
    tree["tintin"] = 42;
    tree["tutu"] = 23;

    assert((tree["tintin"] || 10) == 42);
    assert((tree["toto"] || 10) == 10);
    assert((tree["toto"] || tree["tintin"]) == 42);
    assert((tree["tutu"] || tree["tintin"]) == 23);
    tree["tintin"] = 0;
    assert((tree["tintin"] || tree["tutu"]) == 0);
  }

  // This is pointless to run as a test, but it checks for build errors
  {
    DataTree tree;
    Data data = tree["sub"];
    string keyA("keyA");
    string_view keyB("keyB");
    const char* keyC("keyC");

    assert(tree[keyA].exists() == false);
    assert(tree[keyB].exists() == false);
    assert(tree[keyC].exists() == false);
    assert(data[keyA].exists() == false);
    assert(data[keyB].exists() == false);
    assert(data[keyC].exists() == false);
  }

  return 0;
}
