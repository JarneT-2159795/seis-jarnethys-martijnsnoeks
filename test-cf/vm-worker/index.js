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
  const { searchParams } = new URL(request.url);
  let val1 = parseInt(searchParams.get('num1'));
  let val2 = parseInt(searchParams.get('num2'));
  let result = await useModule(val1, val2);
  return new Response(JSON.stringify({ "output": result} ), { 'status': 200, 'content-type': 'application/json' });
}

const useModule = async (num1, num2) => {
  let instance = await emscripten_module;
  let bts = await FILES.get("code.e082f7f544.wasm");
  bts = new TextEncoder().encode(bts);
  let vars = new Int32Array([num1, num2]);
  let buffer = instance._malloc(bts.length * bts.BYTES_PER_ELEMENT);
  let input = instance._malloc(vars.length * vars.BYTES_PER_ELEMENT);
  instance.HEAPU8.set(bts, buffer);
  instance.HEAP32.set(vars, input >> 2);
  return instance["_useModule"]( buffer, bts.length, input );
}
