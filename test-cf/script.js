<script>
      Module.noInitialRun = true; // prevent main() from being called
      Module.onRuntimeInitialized = function() {
        console.log("Custom runtime for VM started!");
        
        fetch('code.wasm').then(response =>
          response.arrayBuffer()
        ).then(bytes => {
            bts = new Uint8Array(bytes);
            vars = new Int32Array([parseInt(prompt("Enter value 1", "0")), parseInt(prompt("Enter value 2", "0"))]);
            // Allocate some space in the heap for the data (making sure to use the appropriate memory size of the elements)
            let buffer = Module._malloc(bts.length * bts.BYTES_PER_ELEMENT);
            let input = Module._malloc(vars.length * vars.BYTES_PER_ELEMENT);
            // Assign the data to the heap - Keep in mind bytes per element
            Module.HEAPU8.set(bts, buffer); // note: unlike the example, buffer >> 2 didn't work here!!!
            console.log("Gelukt tot hier");
            Module.HEAP32.set(vars, input >> 2);
            console.log(Module);
            Module["_useModule"]( buffer, bts.length, input );
          }
        );
      }
    </script>