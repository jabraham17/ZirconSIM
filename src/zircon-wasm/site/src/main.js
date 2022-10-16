
let dom = {}

dom.stdout = document.querySelector("#stdout textarea");
dom.stderr = document.querySelector("#stderr textarea");


const image_dom = document.querySelector("#fileupload")
document.querySelector("#run-button").addEventListener("click", ev => {
    const files = image_dom.files;
    if (files.length != 1) return;
    let file = files[0];
    let fr = new FileReader();
    fr.onload = function () {
        let data = new Uint8Array(fr.result);
        worker.postMessage({ command: "emulateFile", arguments: [data] });
    };
    fr.readAsArrayBuffer(file);
})

const worker = new Worker('/src/zircon-worker.js');
worker.addEventListener("message", (msg) => {
    if (typeof msg === 'undefined' || !msg) return;
    if (msg.data.command === "emulateFileResults") {
        dom.stdout.innerHTML = msg.data.arguments[0];
        dom.stderr.innerHTML = msg.data.arguments[1];
        scrollTextAreaToBottom(dom.stdout);
        scrollTextAreaToBottom(dom.stderr);
    }
});


