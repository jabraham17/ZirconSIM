
for(let elm of document.querySelectorAll(".grid-layout#main-view>.grid-item textarea ")) {
    elm.addEventListener("input", (e) => {

        console.log(e.target)
    });
}

function scrollTextAreaToBottom(domObj) {
    console.log(domObj)
    domObj.scrollTop = domObj.scrollHeight;
  }
