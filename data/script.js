var webSocketCarInputUrl = "ws:\/\/" + window.location.hostname + "/ws";
var websocketCarInput;
const auxSlider = document.getElementById('AUX');
const bucketSlider = document.getElementById('Bucket');

function initCarInputWebSocket() {
    websocketCarInput = new WebSocket(webSocketCarInputUrl);
    websocketCarInput.onclose = function (event) { setTimeout(initCarInputWebSocket, 2000); };
    websocketCarInput.onmessage = function (event) { };
}

function sendButtonInput(key, value) {
    var data = key + "," + value;
    websocketCarInput.send(data);
}
function handleKeyDown(event) {
    if (event.keyCode === 38) {
        sendButtonInput("MoveCar", "1");
    }
    if (event.keyCode === 40) {
        sendButtonInput("MoveCar", "2");
    }
    if (event.keyCode === 37) {
        sendButtonInput("MoveCar", "3");
    }
    if (event.keyCode === 39) {
        sendButtonInput("MoveCar", "4");
    }
    if (event.keyCode === 87) {
        sendButtonInput("MoveCar", "5");
    }
    if (event.keyCode === 83) {
        sendButtonInput("MoveCar", "6");
    }
}
function handleKeyUp(event) {
    if (event.keyCode === 37 || event.keyCode === 38 || event.keyCode === 39 || event.keyCode === 40 || event.keyCode === 87 || event.keyCode === 83) {
        sendButtonInput("MoveCar", "0");
    }
}


window.onload = initCarInputWebSocket;
document.getElementById("mainTable").addEventListener("touchend", function (event) {
    event.preventDefault()
});
document.addEventListener('keydown', handleKeyDown);
document.addEventListener('keyup', handleKeyUp); 