# JSON Generator
A simple JSON (JavasScript Object Notation) generator with flushing capability.
Details of JSON can be found at [http://www.json.org/](http://www.json.org/).
The JSON strings generated can be validated using any standard JSON validator. Eg. [https://jsonlint.com/](https://jsonlint.com/)

# Files
- `json_generator.c`: Actual source file for the JSON generator with implementation of all APIS
- `json_generator.h`: Header file documenting and exposing all available APIs
- `test.c`: A test app which demonstrates the usage of the JSON generator
- `Makefile`: For generating the test executable

# Usage

Include the C and H files in your project's build system and that should be enough.
`json_generator` requires only standard library functions for compilation

# Testing
- To compile the test executable, just execute "make".
- This will create "json_gen" binary.
- Running the binary should print the expected and generated JSON string on the terminal, and the test result

```text
./json_gen 
Creating JSON string [may require Line wrap enabled on console]
Expected: {"first_bool":true,"first_int":30,"float_val":54.16430,"my_str":"new_name","null_obj":null,"arr":[["arr_string",false,45.12000,null,25,{"arr_obj_str":"sample"}]],"my_obj":{"only_val":5}}
Generated: {"first_bool":true,"first_int":30,"float_val":54.16430,"my_str":"new_name","null_obj":null,"arr":[["arr_string",false,45.12000,null,25,{"arr_obj_str":"sample"}]],"my_obj":{"only_val":5}}
Test Passed!
```

To cleanup the app, execute `make clean`
