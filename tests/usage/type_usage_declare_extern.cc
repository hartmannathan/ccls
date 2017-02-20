struct T {};

extern T t;
/*
OUTPUT:
{
  "types": [{
      "id": 0,
      "usr": "c:@S@T",
      "short_name": "T",
      "qualified_name": "T",
      "definition": "tests/usage/type_usage_declare_extern.cc:1:8",
      "all_uses": ["tests/usage/type_usage_declare_extern.cc:1:8", "tests/usage/type_usage_declare_extern.cc:3:8"]
    }],
  "functions": [],
  "variables": [{
      "id": 0,
      "usr": "c:@t",
      "short_name": "t",
      "qualified_name": "t",
      "declaration": "tests/usage/type_usage_declare_extern.cc:3:10",
      "all_uses": ["tests/usage/type_usage_declare_extern.cc:3:10"]
    }]
}
*/