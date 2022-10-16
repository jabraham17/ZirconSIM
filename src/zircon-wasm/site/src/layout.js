
for (let elm of document.querySelectorAll(".grid-layout#main-view>.grid-item textarea ")) {
    elm.addEventListener("input", (e) => {
        scrollTextAreaToBottom(e.target)
    });
}

function scrollTextAreaToBottom(domObj) {
    domObj.scrollTop = domObj.scrollHeight;
}
