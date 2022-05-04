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
  if (request.method === 'OPTIONS') {
    return handleOptions(request);
  } else {
    if (request.method === 'GET') {
      const { searchParams } = new URL(request.url);
      let result = await useModule(searchParams);
      let response = new Response(result, { 'status': 200, 'content-type': 'application/json' });
      response.headers.set('Access-Control-Allow-Origin', "*");
      response.headers.append('Vary', 'Origin');
      return response;
    } else if (request.method === 'POST') {
      let result = await handlePost(request);
      return result;
    }
  }
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

const useModule = async (params) => {
  let instance = await emscripten_module;

  // get the module
  let bts = new Uint8Array()
  if (params.has('arr')) {
    let recoveredData = JSON.parse(decodeURIComponent(params.get("arr")));
    bts = new Uint8Array(Object.keys(recoveredData).length);
    for (let i = 0; i < bts.length; i++) {
        bts[i] = recoveredData[i.toString()];
    }
  }
  let buffer = instance._malloc(bts.length * bts.BYTES_PER_ELEMENT);

  // get the variables
  let vari32 = new Array();
  let vari64 = new Array();
  let varf32 = new Array();
  let varf64 = new Array();
  for (let i = 1; i < Array.from(params).length + 1; i++) {
    if (params.has("p" + i)) {
      let param = params.get("p" + i);
      let type = params.get("t" + i);
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
  let outi32 = new Int32Array(params.get("oi32"));
  let outi64 = new BigInt64Array(params.get("oi64"));
  let outf32 = new Float32Array(params.get("of32"));
  let outf64 = new Float64Array(params.get("of64"));

  // get the name of the function
  let nameStr = params.get("function")
  let name = new TextEncoder().encode(nameStr);
  if (nameStr == "randomInt") {
    let arr = new Int32Array(1);
    arr[0] = parseInt(instance["_randomInt"]());
    return (JSON.stringify({
      "i32": JSON.stringify(Array.from(arr)),
      "i64": JSON.stringify([]),
      "f32": JSON.stringify([]),
      "f64": JSON.stringify([])
    }))
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
  return (JSON.stringify({
    "i32": JSON.stringify(Array.from(i32OutBuffer)),
    "i64": JSON.stringify(Array.from(i64OutBuffer)),
    "f32": JSON.stringify(Array.from(f32OutBuffer)),
    "f64": JSON.stringify(Array.from(f64OutBuffer))
  }))
}
