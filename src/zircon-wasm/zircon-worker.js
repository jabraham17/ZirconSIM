importScripts("./zircon.js")

// let buffer = ""
// let Module = {
//     print: (s) => buffer += s
// }
let modFactory = Mod()

function emulateFile(data) {
    modFactory.then((mod) => {
        mod.printBuffer = "";
        mod.FS.createDataFile("/", "upload.elf", data, true, false, false);
        mod.emulate("/upload.elf");
        self.postMessage({ command: "emulateFileResults", arguments: [mod.printBuffer] });
        mod.FS.unlink("/upload.elf")
    });
}

self.onmessage = (msg) => {
    if (typeof msg === 'undefined' || !msg) return;
    if (msg.data.command === "emulateFile") {
        emulateFile(...msg.data.arguments);
    }
}
