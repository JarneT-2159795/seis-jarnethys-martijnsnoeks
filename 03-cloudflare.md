---
layout: page
title: Cloudflare deployment
permalink: /cloudflare
---

## Introduction

<a href="https://workers.cloudflare.com/" target="_blank">Cloudflare workers</a> are all machines part of Cloudflare’s Edge Network. This is a distributed network all around the world. These machines use the javascript V8 engine to run thousands of applications. A big advantage here is that our code can be cached by a worker close to us. This means much lower waiting times after our first request. For more information on how you can create a worker for yourself you can visit the <a href="https://developers.cloudflare.com/workers/" target="_blank">Cloudflare developer documentation</a>.

## Limits

We will use the free Cloudflare plan. With this come a few limitations but these won't matter for our usecase.

- A maximum of 30 workers
- Maximum 10 ms CPU time per request
- Limited KV storage

The maximum of 30 workers is no problem because we'll only use one for our project. We can bundle our VM and compiler in one worker. The maximum CPU time is also no problem. We tested our worker by compiling a WAT document and immediately executing a function in this module. As the compilation and execution are both done in one request they have to be less then 10 ms combined. Our worst  case observation measured a CPU time of around 2 ms. Thus we still have plenty of room to expand. The last limitation is the limited use of the KV (key-value) storage. We don't use this feature in our deployment.

## Deploying to a Cloudflare worker

### Cross-compiling to WebAssembly

To cross-compile our VM and compiler we will use <a href="https://emscripten.org/index.html" target="_blank">Emscripten</a>. We create a main.c file in which we write all the functions that we want to use in our worker. We exposed three functions:

- useModule: run functions of an already compiled wasm file
- compile: use WebAssembly text as input to compile and execute a function
- randomInt: a test function to verify that our cross-compiled code works

The code for the whole deployment can be found in our <a href="https://github.com/JarneT-2159795/seis-jarnethys-martijnsnoeks/tree/main/test-cf" target="_blank">Github repository</a>. This is our implementation for the main.c file. We will break down each part.

```c++

#include "../includes/module.h"
#include <iostream>
#include <time.h>
#include "../includes/lexer.h"
#include "../includes/parser.h"
#include "../includes/compiler.h"

extern "C" {
    int useModule(uint8_t *data, int size, char* name, int32_t *int32Input, int64_t *int64Input, float32_t *float32Input, float64_t *float64Input,
                    int32_t *int32Output, int64_t *int64Output, float32_t *float32Output, float64_t *float64Output);
    int compile(uint8_t *data, int size, char *name, int32_t *int32Input, int64_t *int64Input, float32_t *float32Input, float64_t *float64Input,
                    int32_t *int32Output, int64_t *int64Output, float32_t *float32Output, float64_t *float64Output, uint8_t *output, int *outputSize);
    int randomInt() {
        srand(time(0));
        return rand();
    }
}

int compile(uint8_t *data, int size, char *name, int32_t *int32Input, int64_t *int64Input, float32_t *float32Input, float64_t *float64Input,
                int32_t *int32Output, int64_t *int64Output, float32_t *float32Output, float64_t *float64Output, uint8_t *output, int *outputSize) {
    std::vector<uint8_t> chars;
    for (int i = 0; i < size; i++) {
        chars.push_back(data[i]);
    }
	Lexer lexer = Lexer{chars};

    int err = lexer.lex();

    Parser parser = Parser(&lexer);

    std::vector<Instruction*> AST = parser.parseProper();
    auto functions = parser.getFunctions();
    
    Compiler compiler = Compiler(functions);
    auto compiledOutput = compiler.compile();

    uint8_t *outputbuffer = (uint8_t *)malloc(compiledOutput->getTotalByteCount());
    compiledOutput->setByteIndex(0);
    for (int i = 0; i < compiledOutput->getTotalByteCount(); i++) {
        output[i] = compiledOutput->readByte();
    }
    *outputSize = compiledOutput->getTotalByteCount();

    useModule(compiledOutput->getBuffer(), compiledOutput->getTotalByteCount(), name, int32Input, int64Input, float32Input, float64Input,
                int32Output, int64Output, float32Output, float64Output);

    return 0;
}

int useModule(uint8_t *data, int size, char *name, int32_t *int32Input, int64_t *int64Input, float32_t *float32Input, float64_t *float64Input,
                int32_t *int32Output, int64_t *int64Output, float32_t *float32Output, float64_t *float64Output) {
    if (size < 1) {
        std::cout << "Error: No data received" << std::endl;
        return -1;
    }

    std::string nameStr(name);
    if (nameStr == "") {
        std::cout << "Error: No name received" << std::endl;
        return -2;
    }

    int choice = -1;
    ByteStream bs;
    Module module{data, size};
    auto funcs = module.getFunctions();
    for (int i = 0; i < funcs.size(); i++) {
        if (funcs[i].getName() == nameStr) {
            choice = i;
            break;
        }
    }
    if (choice == -1) {
        std::cout << "Error: No function with name " << nameStr << " found" << std::endl;
        return -3;
    }

    Stack vars;
    int i32Count, i64Count, f32Count, f64Count;
    for (int i = 0; i < funcs[choice].getParams().size(); i++) {
        switch (funcs[choice].getParams()[i]) {
            case VariableType::is_int32:
                vars.push(int32Input[i32Count]);
                i32Count++;
                break;
            case VariableType::is_int64:
                vars.push(int64Input[i64Count]);
                i64Count++;
                break;
            case VariableType::isfloat32_t:
                vars.push(float32Input[f32Count]);
                f32Count++;
                break;
            case VariableType::isfloat64_t:
                vars.push(float64Input[f64Count]);
                f64Count++;
                break;
        }
    }

    module(name, vars);
    auto results = module.getResults(funcs[choice].getResults().size());
    for (int i = 0; i < results.size(); i++) {
        switch (funcs[choice].getResults()[i]) {
            case VariableType::is_int32:
                int32Output[i] = std::get<int32_t>(results[i]);
                std::cout << "int32Output[" << i << "] = " << int32Output[i] << std::endl;
                break;
            case VariableType::is_int64:
                int64Output[i] = std::get<int64_t>(results[i]);
                break;
            case VariableType::isfloat32_t:
                float32Output[i] = std::get<float32_t>(results[i]);
                break;
            case VariableType::isfloat64_t:
                float64Output[i] = std::get<float64_t>(results[i]);
                break;
        }
    }
    return 0;
}

```

#### Using `extern "C"`

```c++

extern "C" {
    int useModule(uint8_t *data, int size, char* name, int32_t *int32Input, int64_t *int64Input, float32_t *float32Input, float64_t *float64Input,
                    int32_t *int32Output, int64_t *int64Output, float32_t *float32Output, float64_t *float64Output);

    int compile(uint8_t *data, int size, char *name, int32_t *int32Input, int64_t *int64Input, float32_t *float32Input, float64_t *float64Input,
                    int32_t *int32Output, int64_t *int64Output, float32_t *float32Output, float64_t *float64Output, uint8_t *output, int *outputSize);

    int randomInt() {
        srand(time(0));
        return rand();
    }
}

```

When we use `extern "C"` we define which function we want to export using Emscripten and make available to us in javascript. As you can see above we use **a lot** of pointers. This is because passing variables from and to our code will prove to be less then straightforward and we will use the shared heap for all of them. The functions in `extern "C"` can be defined in-place or we can a forward declaration and define the functions later. We create `randomInt()` in-place as it's just a test function. We use this function to verify that our cross-compiled code works as intended without having to pass through any parameters. Our other two functions are defined later.

#### `useModule()` function

```c++

int useModule(uint8_t *data, int size, char *name, int32_t *int32Input, int64_t *int64Input, float32_t *float32Input, float64_t *float64Input,
                int32_t *int32Output, int64_t *int64Output, float32_t *float32Output, float64_t *float64Output) {
    if (size < 1) {
        std::cout << "Error: No data received" << std::endl;
        return -1;
    }

    std::string nameStr(name);
    if (nameStr == "") {
        std::cout << "Error: No name received" << std::endl;
        return -2;
    }

    int choice = -1;
    ByteStream bs;
    Module module{data, size};
    auto funcs = module.getFunctions();
    for (int i = 0; i < funcs.size(); i++) {
        if (funcs[i].getName() == nameStr) {
            choice = i;
            break;
        }
    }
    if (choice == -1) {
        std::cout << "Error: No function with name " << nameStr << " found" << std::endl;
        return -3;
    }

    Stack vars;
    int i32Count, i64Count, f32Count, f64Count;
    for (int i = 0; i < funcs[choice].getParams().size(); i++) {
        switch (funcs[choice].getParams()[i]) {
            case VariableType::is_int32:
                vars.push(int32Input[i32Count]);
                i32Count++;
                break;
            case VariableType::is_int64:
                vars.push(int64Input[i64Count]);
                i64Count++;
                break;
            case VariableType::isfloat32_t:
                vars.push(float32Input[f32Count]);
                f32Count++;
                break;
            case VariableType::isfloat64_t:
                vars.push(float64Input[f64Count]);
                f64Count++;
                break;
        }
    }

    module(name, vars);
    auto results = module.getResults(funcs[choice].getResults().size());
    for (int i = 0; i < results.size(); i++) {
        switch (funcs[choice].getResults()[i]) {
            case VariableType::is_int32:
                int32Output[i] = std::get<int32_t>(results[i]);
                std::cout << "int32Output[" << i << "] = " << int32Output[i] << std::endl;
                break;
            case VariableType::is_int64:
                int64Output[i] = std::get<int64_t>(results[i]);
                break;
            case VariableType::isfloat32_t:
                float32Output[i] = std::get<float32_t>(results[i]);
                break;
            case VariableType::isfloat64_t:
                float64Output[i] = std::get<float64_t>(results[i]);
                break;
        }
    }
    return 0;
}

```

We can see our function has quite a lot of parameters. We will explain what each one does here:

- `uint8_t *data` and `int size`: this is the array with bytes that make up the module of which we want to execute a function and the size of this array
- `char *name`: the name of the function we want to use
- `int32_t *int32Input`, `int64_t *int64Input`, `float32_t *float32Input` and `float64_t *float64Input`: the input parameters for all four datatypes
- `int32_t *int32Output`, `int64_t *int64Output`, `float32_t *float32Output` and `float64_t *float64Output`: the output parameters for all four datatypes

Before we will try to execute the function we will perform some basic checking of the input. We verify the length of the byte array to make sure we actually received something and we check whether or not we received a function name. After this we create a Module of the byte array and check if the passed function name is present in the Module. After this we check which parameters are required by the function and read these from the shared heap. Note that we don't check if these variables are actually passed. It would involve a lot more parameters to be passed and we assume for the purpose of this deployment that the user will pass the necessary parameters. Finally we can execute our function in the module.

When our function completes we want to read the result and pass them back to javascript so we can return them to the user. As explained later in the javascript section the necessary memory for the result is already malloced. We can just write the result in the already provided location on the shared heap.

#### `compile()` function

```c++

int compile(uint8_t *data, int size, char *name, int32_t *int32Input, int64_t *int64Input, float32_t *float32Input, float64_t *float64Input,
                int32_t *int32Output, int64_t *int64Output, float32_t *float32Output, float64_t *float64Output, uint8_t *output, int *outputSize) {
    std::vector<uint8_t> chars;
    for (int i = 0; i < size; i++) {
        chars.push_back(data[i]);
    }
	Lexer lexer = Lexer{chars};

    int err = lexer.lex();

    Parser parser = Parser(&lexer);

    std::vector<Instruction*> AST = parser.parseProper();
    auto functions = parser.getFunctions();
    
    Compiler compiler = Compiler(functions);
    auto compiledOutput = compiler.compile();

    uint8_t *outputbuffer = (uint8_t *)malloc(compiledOutput->getTotalByteCount());
    compiledOutput->setByteIndex(0);
    for (int i = 0; i < compiledOutput->getTotalByteCount(); i++) {
        output[i] = compiledOutput->readByte();
    }
    *outputSize = compiledOutput->getTotalByteCount();

    useModule(compiledOutput->getBuffer(), compiledOutput->getTotalByteCount(), name, int32Input, int64Input, float32Input, float64Input,
                int32Output, int64Output, float32Output, float64Output);

    return 0;
}

```

The `compile()` function requires two extra parameters in addition to the parameters already required for the `useModule()` function.

- `uint8_t *output` and `int *outputSize`: this is the space used to place the compiled byte array and the size of this array to pass back through to javascript. This space is also already malloced as explained later.

This function is also pretty straightforward as we can copy-paste the code we used to compile a WAT file in our <a href="https://jarnet-2159795.github.io/seis-jarnethys-martijnsnoeks/wasm-compiler" target="_blank">previous blogpost</a>. We first use the lexer to create tokens from the WAT text. This also removes any comments. The parser uses these tokens to create an <a href="https://en.wikipedia.org/wiki/Abstract_syntax_tree" target="_blank">abstract syntax tree (AST)</a>. This makes it much easier to actually compile in the last stage. When we have compiled our WAT text to a byte array we can simply pass this array and all needed parameters to the previously defined `useModule()` function to run the desired function.

#### Compiling to WebAssembly

As explained above we will use Emscripten to compile our project to a single wasm file. 

```bash

em++ ../main.cpp ../../includes/*.cpp -std=c++2a --no-entry -O2 -s WASM=1 -s EXPORTED_FUNCTIONS="[_useModule, _randomInt, _compile]" -s ALLOW_MEMORY_GROWTH=1 -s DYNAMIC_EXECUTION=0 -s TEXTDECODER=0 -s MODULARIZE=1 -s ENVIRONMENT='web' -s EXPORT_NAME="WASMModule" --pre-js '../pre.js' -o test.js

```

We use `em++` on the command line and it can be used like `g++`. We have to pass all files that will be used and also pass extra compile flags.

- `-std=c++2a`: compile using the C++20 standard. We use some new features like `std::variable`
- `--no-entry`: we don't have a main function that has to run on startup
- `-O2`: optimize as much as possible. When experiencing bugs it is recommended to use less/no optimizations
- `-s WASM=1`: our code has to be compiled to WA. When using =0 the target is javascript and =2 creates code for both
- `-s EXPORTED_FUNCTIONS="[_useModule, _randomInt, _compile]"`: list all the functions that have to be exported. All function names require a "_"-prefix
- `-s ALLOW_MEMORY_GROWTH=1`: enable the growing of memory. Otherwise the program would crash when requiring more memory then initially allocated
- `-s DYNAMIC_EXECUTION=0`: allow the code to be executed in places that disallow dynamic code execution
- `-s TEXTDECODER=0`: do not use the javascript TextDecoder API for string marshalling
- `-s MODULARIZE=1`: create a function that returns a promise to create the module in javascript
- `-s EXPORT_NAME="WASMModule"`: the name of the function to create a module
- `-s ENVIRONMENT='web'`: set the runtime environment for the javascript code to web
- `--pre-js '../pre.js'`: any javascript code we want to add to the output. This code will also be optimized by Emscripten
- `-o test.js`: specify the output file

For more information on all the different parameters you can visit <a href="https://github.com/emscripten-core/emscripten/blob/main/src/settings.js" target="_blank">this documentation</a>. The command above will create two files: test.js and test.wasm. We will use these to create our worker.

### Creating a Cloudflare worker

#### Setting up a project

We assume you already created a Cloudflare account and installed the wrangler CLI. For more information how to set this all up you can visit the <a href="https://developers.cloudflare.com/workers/" target="_blank">Cloudflare documentation</a>. Now it's time to create our worker. This an be done with the following command:

```bash

wrangler generate PROJECT_NAME https://github.com/cloudflare/worker-emscripten-template

```

This will provide us with all the necessary files to deploy our WebAssembly project to a worker. With the `wrangler whoami` command you can obtain your account id and enter this in the `wrangler.toml` file.

By default this template will use a docker container to compile the project but we have a pretty simple project so we can adjust the `webpack.config.js` file. We will remove the section which calls `build.js` and copy over our `test.js` and `test.wasm ` files we created earlier to the `/build` directory.

```js

const CopyPlugin = require('copy-webpack-plugin')
const path = require('path')
const spawn = require('child_process').spawnSync

module.exports = {
  context: path.resolve(__dirname, '.'),
  devtool: 'nosources-source-map',
  entry: './index.js',
  target: 'webworker',
  plugins: [
    new CopyPlugin([
      { from: './build/test.wasm', to: './worker/test.wasm' },
    ]),
  ],
  module: {
    rules: [
      {
        test: /emscripten\.js$/,
        loader: 'exports-loader',
      },
    ],
  },
}


```

With all this done we can create the code that will actually run on the worker. This is located in the `index.js` file. One important thing that should be changed immediately at line 19 is the name of the instance. The original is 

```js

let instance = new WebAssembly.Instance(wasm, info)

```

and should be 

```js

let instance = new WebAssembly.Instance(WASM_MODULE, info)

```

Otherwise your WebAssembly file cannot get loaded. As you can see there is already a function `handleRequest(event)`. This function will handle every request we receive. We changed this function completely to fit our needs. We will use GET requests to provide a GUI for our application. This GUI will use POST requests to actually execute our application. For this we need a couple of things:

- A function to handle CORS requests
- HTML documents to provide a basic GUI to our users
- Functions to communicate with our application in WebAssembly

```js

async function handleRequest(event) {
  let request = event.request;
  let url = request.url;
  if (request.method === 'OPTIONS') {
    return handleOptions(request);
  } else {
    if (request.method === 'GET') {
      if (url.includes("compile")) {
        return new Response(watHTML, { headers: { 'content-type': 'text/html;charset=UTF-8', }, });
      } else {
        return new Response(wasmHTML, { headers: { 'content-type': 'text/html;charset=UTF-8', }, });
      }
    } else if (request.method === 'POST') {
      let body = await (request.clone()).text();
      body = JSON.parse(body);
      if (body["wat"] != undefined) {
        let result = await HandlePostCompile(request);
        return result;
      } else {
        let result = await handlePost(request);
        return result;
      }
    }
  }
}

```

#### Handling CORS requests

For this we basically copied the code in the <a href="https://developers.cloudflare.com/workers/examples/cors-header-proxy/" target="_blank">Cloudflare documentation on CORS requests</a>.

```js

const corsHeaders = {
  'Access-Control-Allow-Origin': '*',
  'Access-Control-Allow-Methods': 'GET,HEAD,POST,OPTIONS',
  'Access-Control-Max-Age': '86400',
};

function handleOptions(request) {
  // Make sure the necessary headers are present
  // for this to be a valid pre-flight request
  let headers = request.headers;
  if (
    headers.get('Origin') !== null &&
    headers.get('Access-Control-Request-Method') !== null &&
    headers.get('Access-Control-Request-Headers') !== null
  ) {
    // Handle CORS pre-flight request.
    // If you want to check or reject the requested method + headers
    // you can do that here.
    let respHeaders = {
      ...corsHeaders,
      // Allow all future content Request headers to go back to browser
      // such as Authorization (Bearer) or X-Client-Name-Version
      'Access-Control-Allow-Headers': request.headers.get('Access-Control-Request-Headers'),
    };

    return new Response(null, {
      headers: respHeaders,
    });
  } else {
    // Handle standard OPTIONS request.
    // If you want to allow other HTTP Methods, you can do that here.
    console.log('Standard OPTIONS request');
    return new Response(null, {
      headers: {
        Allow: 'GET, HEAD, POST, OPTIONS',
      },
    });
  }
}

```

As you can see this implementation allows all requests from all origins. You can change this of you want to restrict this to certain domains. This also means that we cannot use the `wrangler dev` command anymore. Previously we could use this to test the project locally instead of pushing it to our worker but CORS doesn't work with a local deployment.

#### Providing a GUI to the user

We can return HTML documents in response to a GET request. We have two different HTML documents: one for use with already a compiled WebAssembly file and one for use with the compiler. These documents are saved as a string in a global variable. Because of this it is important that all javascript for the client side is inside the HTML document. Below you can find our HTML documents.

##### **HTML for pre-compiled wasm file**

```html

<!DOCTYPE html>
<html lang="en"> 
    <head> 
        <title>SEIS-UI</title> 
        <meta charset="utf-8"> 
        <script> 
            async function setInputs() { 
                document.getElementById("input").innerHTML = ""; 
                let inputs = ""; 
                for (let i = 1; i < parseInt(document.getElementById("numInputs").value) + 1; i++) { 
                    inputs += "Input "+ i + ": <input type='text' id='input" + i + "'/><select id='varType" + i + "'><option value='int32'>int32</option><option value='int64'>int64</option><option value='float32'>float32</option><option value='float64'>float64</option></select><br/>"; 
                } 
                document.getElementById("input").innerHTML = inputs; 
            } 
            
            async function runFunction() { 
                let xhr = new XMLHttpRequest(); 
                xhr.open("POST", "https://vm-worker.jarne-thys.workers.dev/"); 
                xhr.setRequestHeader("Accept", "application/json"); 
                xhr.setRequestHeader("Content-Type", "text/plain"); 
                xhr.setRequestHeader('Access-Control-Allow-Headers', '*'); 
                xhr.onload = () => { 
                    data = JSON.parse(xhr.responseText); 
                    outText = ""; 
                    i32Arr = JSON.parse(data["i32"]); 
                    i64Arr = JSON.parse(data["i64"]); 
                    f32Arr = JSON.parse(data["f32"]); 
                    f64Arr = JSON.parse(data["f64"]); 
                    outText += "i32: "; 
                    for (let i = 0; i < i32Arr.length; i++) { 
                        outText += i32Arr[i] + " "; 
                    } 
                    outText += "\\ni64: "; 
                    for (let i = 0; i < i64Arr.length; i++) { 
                        outText += i64Arr[i] + " "; 
                    } 
                    outText += "\\nf32: "; 
                    for (let i = 0; i < f32Arr.length; i++) { 
                        outText += f32Arr[i] + " "; 
                    } 
                    outText += "\\nf64: "; 
                    for (let i = 0; i < f64Arr.length; i++) { 
                        outText += f64Arr[i] + " "; 
                    } 
                    document.getElementById("output").innerHTML = outText; 
                }; 
                let data = { 
                    "function": document.getElementById("functionName").value, 
                    "inputs": parseInt(document.getElementById("numInputs").value) + 1, 
                    "oi32": document.getElementById("outint32").value, 
                    "oi64": document.getElementById("outint64").value, 
                    "of32": document.getElementById("outfloat32").value, 
                    "of64": document.getElementById("outfloat64").value, 
                }; 
                
                for (let i = 1; i < parseInt(document.getElementById("numInputs").value) + 1; i++) { 
                    data["p" + i] = document.getElementById("input" + i).value; 
                    data["t" + i] = document.getElementById("varType" + i).value; 
                } 
                
                let file = document.getElementById("file").files[0];
                if (file) { 
                    let arr = new Blob([file]); 
                    let reader = new FileReader(); 
                    reader.readAsArrayBuffer(arr); 
                    reader.onload = function() { 
                        let buffer = reader.result; 
                        let array = new Uint8Array(buffer); 
                        data["arr"] = array; xhr.send(JSON.stringify(data)); 
                    } 
                } else { 
                    xhr.send(JSON.stringify(data));
                } 
            } 
        </script> 
    </head> 
    <body style="padding-left: 200px;"> 
        <h1>SEIS-UI</h1> Function: <input type="text" id="functionName" value="randomInt"/><br/><br/> 
        Amount of inputs: <input type="number" id="numInputs" value="0" onchange="setInputs()"/><br/><br/> 
        <span id="input"></span><br/> 
        int32 outputs: <input type="number" id="outint32" value="1"/><br/> 
        int64 outputs: <input type="number" id="outint64" value="0"/><br/> 
        float32 outputs: <input type="number" id="outfloat32" value="0"/><br/> 
        float64 outputs: <input type="number" id="outfloat64" value="0"/><br/><br/> 
        <input type="file" id="file" accept=".wasm"/><br/><br/> 
        <input type="button" onclick="runFunction()" value="Run function"/><br/> 
        <textarea id="output" rows="4" cols="40" readonly></textarea> 
    </body>
</html>

```

##### **HTML for compilation of WAT text**

```html

<!DOCTYPE html>
<html lang="en"> 
    <head> 
        <title>SEIS-UI</title>
        <meta charset="utf-8"> 
        <link rel="shortcut icon" href="/favicon.ico" type="image/vnd.microsoft.icon"> 
        <script> 
            async function setInputs() { 
                document.getElementById("input").innerHTML = ""; 
                let inputs = ""; 
                for (let i = 1; i < parseInt(document.getElementById("numInputs").value) + 1; i++) { 
                    inputs += "Input "+ i + ": <input type='text' id='input" + i + "'/><select id='varType" + i + "'><option value='int32'>int32</option><option value='int64'>int64</option><option value='float32'>float32</option><option value='float64'>float64</option></select><br/>";
                } 
                document.getElementById("input").innerHTML = inputs; 
                
            } 

            async function runFunction() { 
                let xhr = new XMLHttpRequest(); 
                xhr.open("POST", "https://vm-worker.jarne-thys.workers.dev/"); 
                xhr.setRequestHeader("Accept", "application/json"); 
                xhr.setRequestHeader("Content-Type", "text/plain"); 
                xhr.setRequestHeader('Access-Control-Allow-Headers', '*'); 

                xhr.onload = () => { 
                    data = JSON.parse(xhr.responseText); 
                    outText = ""; 
                    i32Arr = JSON.parse(data["i32"]); 
                    i64Arr = JSON.parse(data["i64"]); 
                    f32Arr = JSON.parse(data["f32"]); 
                    f64Arr = JSON.parse(data["f64"]);
                    bytes = new Uint8Array(JSON.parse(data["bytes"]));
                    outText += "i32: "; 
                    for (let i = 0; i < i32Arr.length; i++) { 
                        outText += i32Arr[i] + " "; 
                    } 
                    outText += "\\ni64: "; 
                    for (let i = 0; i < i64Arr.length; i++) { 
                        outText += i64Arr[i] + " "; 
                    } 
                    outText += "\\nf32: "; 
                    for (let i = 0; i < f32Arr.length; i++) { 
                        outText += f32Arr[i] + " "; 
                    } 
                    outText += "\\nf64: "; 
                    for (let i = 0; i < f64Arr.length; i++) { 
                        outText += f64Arr[i] + " "; 
                    }
                    document.getElementById("output").innerHTML = outText; 

                    if (document.getElementById("download").checked == true) {
                        let blob = new Blob([bytes], { type: "application/wasm" });
                        let file = new File([bytes], "output.wasm", {
                            type: blob.type,
                            lastModified: new Date().getTime()
                        })
                        let blobUrl = URL.createObjectURL(file);
                        window.location.replace(blobUrl);
                    }
                };

                let data = { 
                    "function": document.getElementById("functionName").value, 
                    "inputs": parseInt(document.getElementById("numInputs").value) + 1, 
                    "oi32": document.getElementById("outint32").value, 
                    "oi64": document.getElementById("outint64").value, 
                    "of32": document.getElementById("outfloat32").value, 
                    "of64": document.getElementById("outfloat64").value, 
                }; 
                    
                for (let i = 1; i < parseInt(document.getElementById("numInputs").value) + 1; i++) { 
                    data["p" + i] = document.getElementById("input" + i).value; 
                    data["t" + i] = document.getElementById("varType" + i).value; 
                } 
                    
                data["wat"] = document.getElementById("wat").value;
                xhr.send(JSON.stringify(data)); 
            }
        </script> 
    </head> 
    <body style="padding-left: 200px;"> 
        <h1>SEIS-UI</h1> 
        Function: <input type="text" id="functionName" value="randomInt"/><br/><br/>
        Amount of inputs: <input type="number" id="numInputs" value="0" onchange="setInputs()"/><br/><br/> 
        <span id="input"></span><br/> 
        int32 outputs: <input type="number" id="outint32" value="1"/><br/> 
        int64 outputs: <input type="number" id="outint64" value="0"/><br/> 
        float32 outputs: <input type="number" id="outfloat32" value="0"/><br/> 
        float64 outputs: <input type="number" id="outfloat64" value="0"/><br/><br/>
        <textarea id="wat" rows="10" cols="100"></textarea><br/><br/> 
        <input type="checkbox" id="download">Download wasm<br/>
        <input type="button" onclick="runFunction()" value="Run function"/><br/>
        <textarea id="output" rows="4" cols="40" readonly></textarea> 
    </body>
</html>

```

The actual HTML body of both functions is pretty similar. The only difference is the input method. When the user already has a wasm file it can be uploaded and send to the worker. The form for the compiler is slightly different. It provides a text area for the WAT text and a checkbox to choose to download the compiled file. Both HTML documents can be copy-pasted to the `index.js` file. Paste them between two ticks (e.g. \`HTML HERE\`) in order to create a multiline string that ignores ' and " characters as the end of the string.

We also included two javascript functions in the HTML document: one for updating the input variables and one for actually sending a POST request to the worker. The important one is obviously the one sending the POST request and we will examine it in more depth.

```js

async function runFunction() { 
    let xhr = new XMLHttpRequest(); 
    xhr.open("POST", "https://vm-worker.jarne-thys.workers.dev/"); 
    xhr.setRequestHeader("Accept", "application/json"); 
    xhr.setRequestHeader("Content-Type", "text/plain"); 
    xhr.setRequestHeader('Access-Control-Allow-Headers', '*'); 

    xhr.onload = () => { 
        data = JSON.parse(xhr.responseText); 
        outText = ""; 
        i32Arr = JSON.parse(data["i32"]); 
        i64Arr = JSON.parse(data["i64"]); 
        f32Arr = JSON.parse(data["f32"]); 
        f64Arr = JSON.parse(data["f64"]);
        bytes = new Uint8Array(JSON.parse(data["bytes"]));
        outText += "i32: "; 
        for (let i = 0; i < i32Arr.length; i++) { 
            outText += i32Arr[i] + " "; 
        } 
        outText += "\\ni64: "; 
        for (let i = 0; i < i64Arr.length; i++) { 
            outText += i64Arr[i] + " "; 
        } 
        outText += "\\nf32: "; 
        for (let i = 0; i < f32Arr.length; i++) { 
            outText += f32Arr[i] + " "; 
        } 
        outText += "\\nf64: "; 
        for (let i = 0; i < f64Arr.length; i++) { 
            outText += f64Arr[i] + " "; 
        }
        document.getElementById("output").innerHTML = outText; 

        if (document.getElementById("download").checked == true) {
            let blob = new Blob([bytes], { type: "application/wasm" });
            let file = new File([bytes], "output.wasm", {
                type: blob.type,
                lastModified: new Date().getTime()
            })
            let blobUrl = URL.createObjectURL(file);
            window.location.replace(blobUrl);
        }
    };

    let data = { 
        "function": document.getElementById("functionName").value, 
        "inputs": parseInt(document.getElementById("numInputs").value) + 1, 
        "oi32": document.getElementById("outint32").value, 
        "oi64": document.getElementById("outint64").value, 
        "of32": document.getElementById("outfloat32").value, 
        "of64": document.getElementById("outfloat64").value, 
    }; 
        
    for (let i = 1; i < parseInt(document.getElementById("numInputs").value) + 1; i++) { 
        data["p" + i] = document.getElementById("input" + i).value; 
        data["t" + i] = document.getElementById("varType" + i).value; 
    } 
        
    data["wat"] = document.getElementById("wat").value;
    xhr.send(JSON.stringify(data)); 
}

```

As you can see we will be using a XMLHttpRequest to send requests to our worker. The `onload()` function is the function that will be executed when the request is successfully completed. We simply parse each key-value pair. Every value is an array of return values. We have an array for each type of return value and an array for the bytes of the compiled wasm file if we are using the compiler. We parse these to show them in the output text field. If the Download wasm-checkbox is checked we use the bytes to create a downloadable wasm file. The data we send is also encoded as JSON. We have an attribute for the name of the function, the number of inputs and the amount of results for each datatype. Furthermore we have an attribute for each parameters value and datatype. If we are using the compiler we have an attribute "wat" whose value is the raw input string. If we are using a pre-compiled wasm file we convert the file to a `Uint8Array` and send the array. Now that we have send our request to our worker it's time to look how we can handle the request and pass them to our WebAssembly code.

#### Handling the POST requests

We have two functions to handle POST requests. One which uses the compiler and one which uses the `useModule()` function directly. We will explain the one which uses the compiler in depth here because it is basically the same as the other one with some added variables to get the output bytes. We will split the function in smaller pieces and explain it bit by bit.

```js

async function HandlePostCompile(request) {
  let body = await request.text();
  body = JSON.parse(body);
  let instance = await emscripten_module;

  // get the module
  let encoder = new TextEncoder();
  let bts = encoder.encode(body["wat"]);
  let buffer = instance._malloc(bts.length * bts.BYTES_PER_ELEMENT);

  // get the variables
  let vari32 = new Array();
  let vari64 = new Array();
  let varf32 = new Array();
  let varf64 = new Array();
  for (let i = 1; i < body["inputs"]; i++) {
    if (body["p" + i] != undefined) {
      let param = body["p" + i];
      let type = body["t" + i];
      switch (type) {
        case "int32":
          vari32.push(parseInt(param));
          break;
        case "int64":
          vari64.push(BigInt(param));
          break;
        case "float32":
          varf32.push(parseFloat(param));
          break;
        case "float64":
          varf64.push(parseFloat(param));
          break;
        default:
          console.log("Unsupported type: " + type);
          return("Unsupported type: " + type);
      }
    }
  }
  vari32 = new Int32Array(vari32);
  vari64 = new BigInt64Array(vari64);
  varf32 = new Float32Array(varf32);
  varf64 = new Float64Array(varf64);

  // get the output types
  let outi32 = new Int32Array(body["oi32"]);
  let outi64 = new BigInt64Array(body["oi64"]);
  let outf32 = new Float32Array(body["of32"]);
  let outf64 = new Float64Array(body["of64"]);

  // get the name of the function
  let nameStr = body["function"] + "\0";
  console.log(nameStr);
  let name = new TextEncoder().encode(nameStr);
  console.log(name);
  if (nameStr == "randomInt") {
    let arr = new Int32Array(1);
    arr[0] = parseInt(instance["_randomInt"]());
    let result = (JSON.stringify({
      "i32": JSON.stringify(Array.from(arr)),
      "i64": JSON.stringify([]),
      "f32": JSON.stringify([]),
      "f64": JSON.stringify([])
    }));
    let response = new Response(result, { 'status': 200, 'content-type': 'text/plain' });
    response.headers.set('Access-Control-Allow-Origin', "*");
    response.headers.append('Vary', 'Origin');
    return response;
  }

  // assign memory for all inputs and outputs
  // / BYTES: https://gist.github.com/aknuds1/533f7b228aa46e9ee4c8
  let i32buffer = -1;
  if (vari32.length > 0) {
    i32buffer = instance._malloc(vari32.length * vari32.BYTES_PER_ELEMENT);
    instance.HEAP32.set(vari32, i32buffer / vari32.BYTES_PER_ELEMENT);
  }
  let i64buffer = -1;
  if (vari64.length > 0) {
    i64buffer = instance._malloc(vari64.length * vari64.BYTES_PER_ELEMENT);
    instance.HEAP64.set(vari64, i64buffer / vari64.BYTES_PER_ELEMENT);
  }
  let f32buffer = -1;
  if (varf32.length > 0) {
    f32buffer = instance._malloc(varf32.length * varf32.BYTES_PER_ELEMENT);
    instance.HEAPF32.set(varf32, f32buffer / varf32.BYTES_PER_ELEMENT);
  }
  let f64buffer = -1;
  if (varf64.length > 0) {
    f64buffer = instance._malloc(varf64.length * varf64.BYTES_PER_ELEMENT);
    instance.HEAPF64.set(varf64, f64buffer / varf64.BYTES_PER_ELEMENT);
  }
  let i32OutBuffer = -1;
  if (outi32.length > 0) {
    i32OutBuffer = instance._malloc(outi32.length * outi32.BYTES_PER_ELEMENT);
  }
  let i64OutBuffer = -1;
  if (outi64.length > 0) {
    i64OutBuffer = instance._malloc(outi64.length * outi64.BYTES_PER_ELEMENT);
  }
  let f32OutBuffer = -1;
  if (outf32.length > 0) {
    f32OutBuffer = instance._malloc(outf32.length * outf32.BYTES_PER_ELEMENT);
  }
  let f64OutBuffer = -1;
  if (outf64.length > 0) {
    f64OutBuffer = instance._malloc(outf64.length * outf64.BYTES_PER_ELEMENT);
  }

  let bytesOut = instance._malloc(200000);
  let bytesOutCount = instance._malloc(1 * 4);

  let nameBuffer = instance._malloc(name.length * name.BYTES_PER_ELEMENT);
  instance.HEAPU8.set(bts, buffer);
  instance.HEAPU8.set(name, nameBuffer);

  instance["_compile"](buffer, bts.length, nameBuffer, i32buffer, i64buffer, f32buffer, f64buffer, 
                                i32OutBuffer, i64OutBuffer, f32OutBuffer, f64OutBuffer, bytesOut, bytesOutCount);
  
  if (i32OutBuffer != -1) {
    i32OutBuffer = new Int32Array(instance.HEAP32.buffer, i32OutBuffer, outi32.length);
  }
  if (i64OutBuffer != -1) {
    i64OutBuffer = new BigInt64Array(instance.HEAP64.buffer, i64OutBuffer, outi64.length);
  }
  if (f32OutBuffer != -1) {
    f32OutBuffer = new Float32Array(instance.HEAPF32.buffer, f32OutBuffer, outf32.length);
  }
  if (f64OutBuffer != -1) {
    f64OutBuffer = new Float64Array(instance.HEAPF64.buffer, f64OutBuffer, outf64.length);
  }

  bytesOutCount = new Int32Array(instance.HEAP32.buffer, bytesOutCount, 1);
  let bytesOutBuffer = new Uint8Array(instance.HEAPU8.buffer, bytesOut, bytesOutCount[0]);

  let resultJson = JSON.stringify({
    "i32": JSON.stringify(Array.from(i32OutBuffer)),
    "i64": JSON.stringify(Array.from(i64OutBuffer)),
    "f32": JSON.stringify(Array.from(f32OutBuffer)),
    "f64": JSON.stringify(Array.from(f64OutBuffer)),
    "bytes": JSON.stringify(Array.from(bytesOutBuffer))
  });

  let response = new Response(resultJson, { 'status': 200, 'content-type': 'text/plain' });
  response.headers.set('Access-Control-Allow-Origin', "*");
  response.headers.append('Vary', 'Origin');
  return response;
}

```

##### **Getting all variables**

To get all variables we first parse the body to a JSON object. We also create an emscripten_module. This module already contains the code which we cross-compiled earlier. We first get the "wat"-attribute from the JOn object. This contains the raw input string for the compiler. Using the javascript TextEncoder API we can easily create a `Uint8Array` from the text. After this we get our first experience with the Emscripten interface. We can now use `malloc()` in javascript! We reserve the amount of bytes needed to store our array that we just created.

After this we parse all of our parameters and use their datatypes to store them in the appropriate array. We also create arrays for the outputs. These already have the correct size but have no values assigned yet. The last input parameter we need is the name of the function we are going to execute. If it is randomInt we can immediately return the output value but we will not use it now.

```js

let body = await request.text();
body = JSON.parse(body);
let instance = await emscripten_module;

// get the module
let encoder = new TextEncoder();
let bts = encoder.encode(body["wat"]);
let buffer = instance._malloc(bts.length * bts.BYTES_PER_ELEMENT);

// get the variables
let vari32 = new Array();
let vari64 = new Array();
let varf32 = new Array();
let varf64 = new Array();
for (let i = 1; i < body["inputs"]; i++) {
if (body["p" + i] != undefined) {
    let param = body["p" + i];
    let type = body["t" + i];
    switch (type) {
    case "int32":
        vari32.push(parseInt(param));
        break;
    case "int64":
        vari64.push(BigInt(param));
        break;
    case "float32":
        varf32.push(parseFloat(param));
        break;
    case "float64":
        varf64.push(parseFloat(param));
        break;
    default:
        console.log("Unsupported type: " + type);
        return("Unsupported type: " + type);
    }
}
}
vari32 = new Int32Array(vari32);
vari64 = new BigInt64Array(vari64);
varf32 = new Float32Array(varf32);
varf64 = new Float64Array(varf64);

// get the output types
let outi32 = new Int32Array(body["oi32"]);
let outi64 = new BigInt64Array(body["oi64"]);
let outf32 = new Float32Array(body["of32"]);
let outf64 = new Float64Array(body["of64"]);

// get the name of the function
let nameStr = body["function"] + "\0";
let name = new TextEncoder().encode(nameStr);
if (nameStr == "randomInt") {
    let arr = new Int32Array(1);
    arr[0] = parseInt(instance["_randomInt"]());
    let result = (JSON.stringify({
        "i32": JSON.stringify(Array.from(arr)),
        "i64": JSON.stringify([]),
        "f32": JSON.stringify([]),
        "f64": JSON.stringify([])
    }));
    let response = new Response(result, { 'status': 200, 'content-type': 'text/plain' });
    response.headers.set('Access-Control-Allow-Origin', "*");
    response.headers.append('Vary', 'Origin');
    return response;
}

```

##### **Reserving memory for all variables**

Now that we have all parameters and know which results we can expect it's time to reserve memory an the shared heap to pass everything to our WebAssembly binary. This might seem simple but there is one key difference between memory in WebAssembly and memory in C++. WA uses indexes and C++ uses an offset in bytes (see the image below). For this example we wil assume that we wan't to allocate memory for a 32 bit integer and that that memory for another 32 bit integer is already reserved. This means that bytes 0 - 3 are already used. When we use `malloc()` function and request 4 bytes of memory we get a pointer in bytes because we use a C/C++ function. In this case we would receive 4 as that is the start of the second 32 bit integer. However now we want to use the memory in WebAssembly to actually store our value. For this we divide our pointer by the number of bytes of our datatype, in this case 4. This will result in the index 1 which WebAssembly can use.

![C++ vs WA memory](/assets/memory-diff.png)
*Visualization of two integers in C++ and WebAssembly memory*

```js

// assign memory for all inputs and outputs
// / BYTES: https://gist.github.com/aknuds1/533f7b228aa46e9ee4c8
let i32buffer = -1;
if (vari32.length > 0) {
    i32buffer = instance._malloc(vari32.length * vari32.BYTES_PER_ELEMENT);
    instance.HEAP32.set(vari32, i32buffer / vari32.BYTES_PER_ELEMENT);
}
let i64buffer = -1;
if (vari64.length > 0) {
    i64buffer = instance._malloc(vari64.length * vari64.BYTES_PER_ELEMENT);
    instance.HEAP64.set(vari64, i64buffer / vari64.BYTES_PER_ELEMENT);
}
let f32buffer = -1;
if (varf32.length > 0) {
    f32buffer = instance._malloc(varf32.length * varf32.BYTES_PER_ELEMENT);
    instance.HEAPF32.set(varf32, f32buffer / varf32.BYTES_PER_ELEMENT);
}
let f64buffer = -1;
if (varf64.length > 0) {
    f64buffer = instance._malloc(varf64.length * varf64.BYTES_PER_ELEMENT);
    instance.HEAPF64.set(varf64, f64buffer / varf64.BYTES_PER_ELEMENT);
}
let i32OutBuffer = -1;
if (outi32.length > 0) {
    i32OutBuffer = instance._malloc(outi32.length * outi32.BYTES_PER_ELEMENT);
}
let i64OutBuffer = -1;
if (outi64.length > 0) {
    i64OutBuffer = instance._malloc(outi64.length * outi64.BYTES_PER_ELEMENT);
}
let f32OutBuffer = -1;
if (outf32.length > 0) {
    f32OutBuffer = instance._malloc(outf32.length * outf32.BYTES_PER_ELEMENT);
}
let f64OutBuffer = -1;
if (outf64.length > 0) {
    f64OutBuffer = instance._malloc(outf64.length * outf64.BYTES_PER_ELEMENT);
}

let bytesOut = instance._malloc(200000);
let bytesOutCount = instance._malloc(1 * 4);

let nameBuffer = instance._malloc(name.length * name.BYTES_PER_ELEMENT);
instance.HEAPU8.set(bts, buffer);
instance.HEAPU8.set(name, nameBuffer);

```

##### **Returning the results**

With all our memory allocated we can finally call our compiler and execute the function. We pass all the pointers to C++ and hope for the best. Notice how we extract our results back from WebAssembly memory. We use the `instance.HEAP.buffer` variable for each datatype. Because the buffer is implemented as an array of bytes instead of using indexes as earlier we don't divide the pointer by the number of bytes that make up the datatype. One thing we are not really that proud of is how we handle the return of the compiled bytecode. We don't know how big it'll need to be thus we tried mallocing in C++, return the pointer and read the memory in javascript. Unfortunately we couldn't get this to work so we malloc 200.000 bytes and only use the ones we need. When we have all our results we put them in a JSON object and return this as respoonse to the request.

```js

instance["_compile"](buffer, bts.length, nameBuffer, i32buffer, i64buffer, f32buffer, f64buffer, 
                            i32OutBuffer, i64OutBuffer, f32OutBuffer, f64OutBuffer, bytesOut, bytesOutCount);

if (i32OutBuffer != -1) {
    i32OutBuffer = new Int32Array(instance.HEAP32.buffer, i32OutBuffer, outi32.length);
}
if (i64OutBuffer != -1) {
    i64OutBuffer = new BigInt64Array(instance.HEAP64.buffer, i64OutBuffer, outi64.length);
}
if (f32OutBuffer != -1) {
    f32OutBuffer = new Float32Array(instance.HEAPF32.buffer, f32OutBuffer, outf32.length);
}
if (f64OutBuffer != -1) {
    f64OutBuffer = new Float64Array(instance.HEAPF64.buffer, f64OutBuffer, outf64.length);
}

bytesOutCount = new Int32Array(instance.HEAP32.buffer, bytesOutCount, 1);
let bytesOutBuffer = new Uint8Array(instance.HEAPU8.buffer, bytesOut, bytesOutCount[0]);

let resultJson = JSON.stringify({
    "i32": JSON.stringify(Array.from(i32OutBuffer)),
    "i64": JSON.stringify(Array.from(i64OutBuffer)),
    "f32": JSON.stringify(Array.from(f32OutBuffer)),
    "f64": JSON.stringify(Array.from(f64OutBuffer)),
    "bytes": JSON.stringify(Array.from(bytesOutBuffer))
});

let response = new Response(resultJson, { 'status': 200, 'content-type': 'text/plain' });
response.headers.set('Access-Control-Allow-Origin', "*");
response.headers.append('Vary', 'Origin');
return response;

```
