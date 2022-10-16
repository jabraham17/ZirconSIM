Module["stdinBuffer"] = ""
Module["stdoutBuffer"] = ""
Module["stderrBuffer"] = ""

// remove char from head
String.prototype.removeCharFromHead = () => {
    let tmp = this.split('');
    tmp.splice(0, 1);
    return tmp.join('');
}

Module["preRun"] = () => {
    let stdin = () => {
        if (Module["stdinBuffer"] === "") return null;
        let asciiCode = Module["stdinBuffer"].charCodeAt(0);
        Module["stdinBuffer"] = Module["stdinBuffer"].removeCharFromHead();
        return asciiCode;
    }
    let stdout = (asciiCode) => {
        if (asciiCode === "\n".charCodeAt(0)) {
            Module["stdoutBuffer"] += "&#10;";
        }
        else if (asciiCode === null) {
            // flush
        }
        else {
            Module["stdoutBuffer"] += String.fromCharCode(asciiCode);
        }
    }
    let stderr = (asciiCode) => {
        if (asciiCode === "\n".charCodeAt(0)) {
            Module["stderrBuffer"] += "&#10;";
        } else if (asciiCode === null) {
            // flush
        } else {
            Module["stderrBuffer"] += String.fromCharCode(asciiCode);
        }
    }
    FS.init(stdin, stdout, stderr);
}


Module["onAbort"] = (s) => {
    Module["stderrBuffer"] += "ABORTED: " + s + "&#10;";
}
Module["printErr"] = (s) => {
    Module["stderrBuffer"] += "ERR: " + s + "&#10;";
}
