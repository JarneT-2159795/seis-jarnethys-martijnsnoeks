import WASMModule from './build/test.js'

addEventListener('fetch', event => {
  event.respondWith(handleRequest(event))
});

// this is where the magic happens
// we send our own instantiateWasm function
// to the emscripten module
// so we can initialize the WASM instance ourselves
// since Workers puts your wasm file in global scope
// as a binding. In this case, this binding is called
// `wasm` as that is the name Wrangler uses
// for any uploaded wasm module
let emscripten_module = new Promise((resolve, reject) => {
  WASMModule({
    instantiateWasm(info, receive) {
      try{
        let instance = new WebAssembly.Instance(WASM_MODULE, info);
        receive(instance);
        return instance.exports; 
      }
      catch(e) {
        reject(e);
      }
    }
  }).then(module => {
    resolve(module);
  })
})

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

async function handlePost(request) {
  let body = await request.text();
  body = JSON.parse(body);
  let instance = await emscripten_module;

  // get the module
  let bts = new Uint8Array()
  if (body["arr"] != undefined) {
    let arr = [];
    for (let i in body["arr"]) {
      arr.push(body["arr"][i]);
    }
    bts = new Uint8Array(arr);
  }
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
  let nameStr = body["function"];
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

  let nameBuffer = instance._malloc(name.length * name.BYTES_PER_ELEMENT);
  instance.HEAPU8.set(bts, buffer);
  instance.HEAPU8.set(name, nameBuffer);

  instance["_useModule"](buffer, bts.length, nameBuffer, i32buffer, i64buffer, f32buffer, f64buffer, 
                                i32OutBuffer, i64OutBuffer, f32OutBuffer, f64OutBuffer);
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
  let resultJson = JSON.stringify({
    "i32": JSON.stringify(Array.from(i32OutBuffer)),
    "i64": JSON.stringify(Array.from(i64OutBuffer)),
    "f32": JSON.stringify(Array.from(f32OutBuffer)),
    "f64": JSON.stringify(Array.from(f64OutBuffer))
  });

  let response = new Response(resultJson, { 'status': 200, 'content-type': 'text/plain' });
  response.headers.set('Access-Control-Allow-Origin', "*");
  response.headers.append('Vary', 'Origin');
  return response;
}

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

// worker url: https://vm-worker.jarne-thys.workers.dev/

const wasmHTML = `<!DOCTYPE html><html lang="en"> <head> <title>SEIS-UI</title> <meta charset="utf-8"> <link rel="shortcut icon" href="/favicon.ico" type="image/vnd.microsoft.icon"> <script> async function setInputs() { document.getElementById("input").innerHTML = ""; let inputs = ""; for (let i = 1; i < parseInt(document.getElementById("numInputs").value) + 1; i++) { inputs += "Input "+ i + ": <input type='text' id='input" + i + "'/><select id='varType" + i + "'><option value='int32'>int32</option><option value='int64'>int64</option><option value='float32'>float32</option><option value='float64'>float64</option></select><br/>"; } document.getElementById("input").innerHTML = inputs; } async function runFunction() { let xhr = new XMLHttpRequest(); xhr.open("POST", "https://vm-worker.jarne-thys.workers.dev/"); xhr.setRequestHeader("Accept", "application/json"); xhr.setRequestHeader("Content-Type", "text/plain"); xhr.setRequestHeader('Access-Control-Allow-Headers', '*'); xhr.onload = () => { data = JSON.parse(xhr.responseText); outText = ""; i32Arr = JSON.parse(data["i32"]); i64Arr = JSON.parse(data["i64"]); f32Arr = JSON.parse(data["f32"]); f64Arr = JSON.parse(data["f64"]); outText += "i32: "; for (let i = 0; i < i32Arr.length; i++) { outText += i32Arr[i] + " "; } outText += "\\ni64: "; for (let i = 0; i < i64Arr.length; i++) { outText += i64Arr[i] + " "; } outText += "\\nf32: "; for (let i = 0; i < f32Arr.length; i++) { outText += f32Arr[i] + " "; } outText += "\\nf64: "; for (let i = 0; i < f64Arr.length; i++) { outText += f64Arr[i] + " "; } document.getElementById("output").innerHTML = outText; }; let data = { "function": document.getElementById("functionName").value, "inputs": parseInt(document.getElementById("numInputs").value) + 1, "oi32": document.getElementById("outint32").value, "oi64": document.getElementById("outint64").value, "of32": document.getElementById("outfloat32").value, "of64": document.getElementById("outfloat64").value, }; for (let i = 1; i < parseInt(document.getElementById("numInputs").value) + 1; i++) { data["p" + i] = document.getElementById("input" + i).value; data["t" + i] = document.getElementById("varType" + i).value; } let file = document.getElementById("file").files[0]; if (file) { let arr = new Blob([file]); let reader = new FileReader(); reader.readAsArrayBuffer(arr); reader.onload = function() { let buffer = reader.result; let array = new Uint8Array(buffer); data["arr"] = array; xhr.send(JSON.stringify(data)); } } else { xhr.send(JSON.stringify(data)); } } </script> </head> <body style="padding-left: 200px;"> <h1>SEIS-UI</h1> Function: <input type="text" id="functionName" value="randomInt"/><br/><br/> Amount of inputs: <input type="number" id="numInputs" value="0" onchange="setInputs()"/><br/><br/> <span id="input"></span><br/> int32 outputs: <input type="number" id="outint32" value="1"/><br/> int64 outputs: <input type="number" id="outint64" value="0"/><br/> float32 outputs: <input type="number" id="outfloat32" value="0"/><br/> float64 outputs: <input type="number" id="outfloat64" value="0"/><br/><br/> <input type="file" id="file" accept=".wasm"/><br/><br/> <input type="button" onclick="runFunction()" value="Run function"/><br/> <textarea id="output" rows="4" cols="40" readonly></textarea> </body></html>`;

const watHTML = `<!DOCTYPE html>
<html lang="en"> 
    <head> 
        <title>SEIS-UI</title>
        <meta charset="utf-8"> 
        <link rel="shortcut icon" href="/favicon.ico" type="image/vnd.microsoft.icon"> 
        <script> 
        async function setInputs() { 
            document.getElementById("input").innerHTML = ""; let inputs = ""; for (let i = 1; i < parseInt(document.getElementById("numInputs").value) + 1; i++) { inputs += "Input "+ i + ": <input type='text' id='input" + i + "'/><select id='varType" + i + "'><option value='int32'>int32</option><option value='int64'>int64</option><option value='float32'>float32</option><option value='float64'>float64</option></select><br/>"; } document.getElementById("input").innerHTML = inputs; 
            } 
            async function runFunction() { 
                let xhr = new XMLHttpRequest(); 
                xhr.open("POST", "https://vm-worker.jarne-thys.workers.dev/"); 
                xhr.setRequestHeader("Accept", "application/json"); xhr.setRequestHeader("Content-Type", "text/plain"); 
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
                  for (let i = 0; i < i64Arr.length; i++) { outText += i64Arr[i] + " "; } outText += "\\nf32: "; for (let i = 0; i < f32Arr.length; i++) { outText += f32Arr[i] + " "; } outText += "\\nf64: "; for (let i = 0; i < f64Arr.length; i++) { outText += f64Arr[i] + " "; }
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
                    "oi64": document.getElementById("outint64").value, "of32": document.getElementById("outfloat32").value, "of64": document.getElementById("outfloat64").value, }; 
                    
                for (let i = 1; i < parseInt(document.getElementById("numInputs").value) + 1; i++) { 
                    data["p" + i] = document.getElementById("input" + i).value; data["t" + i] = document.getElementById("varType" + i).value; 
                } 
                    
                data["wat"] = document.getElementById("wat").value;
                xhr.send(JSON.stringify(data)); 
            } 
            </script> 
    </head> 
    <body style="padding-left: 200px;"> 
        <h1>SEIS-UI</h1> 
        Function: <input type="text" id="functionName" value="whileOne"/><br/><br/>
        Amount of inputs: <input type="number" id="numInputs" value="0" onchange="setInputs()"/><br/><br/> 
        <span id="input"></span><br/> 
        int32 outputs: <input type="number" id="outint32" value="1"/><br/> 
        int64 outputs: <input type="number" id="outint64" value="0"/><br/> 
        float32 outputs: <input type="number" id="outfloat32" value="0"/><br/> 
        float64 outputs: <input type="number" id="outfloat64" value="0"/><br/><br/>
        <textarea id="wat" rows="20" cols="100">
(module
  (func (export "whileOne") (result i32) (local $i i32)
        i32.const 5
        local.set $i  ;; i = first input parameter

        (loop
            ;; add one to $i
            local.get $i
            i32.const 1
            i32.add
            local.set $i

            ;; if $i is less than 10, jump back to loop
            local.get $i
            i32.const 10
            i32.lt_s        ;; _s = signed variant of lt
            br_if 0
        )
      
        local.get $i ;; return final value of $i, should be 10
    )
)               
        </textarea><br/><br/> 
        <input type="checkbox" id="download">Download wasm<br/>
        <input type="button" onclick="runFunction()" value="Run function"/><br/>
        <textarea id="output" rows="4" cols="40" readonly></textarea> 
    </body>
</html>`;
