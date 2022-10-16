

const console_dom = document.querySelector('#console');


const image_dom = document.querySelector("#fileupload")
const form_dom = document.querySelector("#form")
document.querySelector("#submit").addEventListener("click", ev => {
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

const worker = new Worker('./zircon-worker.js');
worker.addEventListener("message", (msg) => {
    if (typeof msg === 'undefined' || !msg) return;
    if (msg.data.command === "emulateFileResults") {
        let result = msg.data.arguments[0];
        console_dom.innerHTML = result;
    }
});


