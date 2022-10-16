importScripts("/src/zircon.js")

let modFactory = Mod()

function emulateFile(data) {
    modFactory.then((mod) => {
        try {
            mod.stdoutBuffer = "";
            mod.stderrBuffer = "";
            mod.FS.createDataFile("/", "upload.elf", data, true, false, false);
            mod.emulate("/upload.elf");
        }
        catch (e) {
            let exception_str = e;
            mod.stderrBuffer += "&#10;" + exception_str;
        }
        finally {
            mod.FS.unlink("/upload.elf");
            self.postMessage({ command: "emulateFileResults", arguments: [mod.stdoutBuffer, mod.stderrBuffer] });
        }
    });
}

self.onmessage = (msg) => {
    if (typeof msg === 'undefined' || !msg) return;
    if (msg.data.command === "emulateFile") {
        emulateFile(...msg.data.arguments);
    }
}
