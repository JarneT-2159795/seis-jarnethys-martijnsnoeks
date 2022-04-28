import WASMModule from './build/wat-parser.js'

addEventListener('fetch', event => {
  event.respondWith(handleRequest(event))
})

// this is where the magic happens
// we send our own instantiateWasm function
// to the emscripten module
// so we can initialize the WASM instance ourselves
// since Workers puts your wasm file in global scope
// as a binding. In this case, this binding is called
// `WASM_MODULE` as that is the name Wrangler uses
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

  // expects to be called like: https://seis-wasm-parser-1.rmarx.workers.dev/?wat=(module(func(export "addTwo")(param i32 i32)(result i32) (; test ;) local.get 0 local.get 1 i32.add))
  const { searchParams } = new URL(request.url);
  let watString = searchParams.get('wat');
  console.log(watString);

  if ( !watString || watString == "") {
    watString = "(module)";
  }

  // let result = await parseWAT("(module(func(export \"addTwo\")(param i32 i32)(result i32) (; test ;) local.get 0 local.get 1 i32.add))");
  let result = await parseWAT( watString );
    
  return new Response(JSON.stringify({ "byteCount": result} ), { 'status': 200, 'content-type': 'application/json' });
}

const parseWAT = async watString => {

  let instance = await emscripten_module;

  let u8str = new TextEncoder("ascii").encode(watString);

  // Allocate some space in the heap for the data (making sure to use the appropriate memory size of the elements)
  let buffer = instance._malloc(u8str.length * u8str.BYTES_PER_ELEMENT);

  // Assign the data to the heap - Keep in mind bytes per element
  instance.HEAPU8.set(u8str, buffer); // note: unlike the example, buffer >> 2 didn't work here!!!

  let result = instance["_parseWAT"]( buffer, u8str.length );

  console.log("Amount of bytes in compiled WASM: ", result);

  return result;
}
