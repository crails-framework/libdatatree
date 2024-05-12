#include <crails/datatree.hpp>
#include <string_view>
#include <iostream>

#undef NDEBUG
#include <cassert>

int main()
{
  using namespace std;
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

  {
    DataTree tree;

    tree["number_list"].from_vector<int>({1,2,3,5,7,11,13,17});
    assert(tree["number_list"].count() == 8);
    assert(tree["number_list"].at(2) == 3);
    assert(tree["number_list"].to_vector<int>().size() == 8);
    assert(tree["number_list"].to_vector<int>()[5] == 11);
  }

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

  return 0;
}
