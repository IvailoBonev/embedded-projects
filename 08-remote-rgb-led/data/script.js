// Runs whenever any slider moves
function update() {
  //Get slider values
  var r = document.getElementById("rSlider").value;
  var g = document.getElementById("gSlider").value;
  var b = document.getElementById("bSlider").value;

  //Update numbers next to sliders
  document.getElementById("rVal").textContent = r;
  document.getElementById("gVal").textContent = g;
  document.getElementById("bVal").textContent = b;

  // Update the live color preview box
  document.getElementById("preview").style.background =
    "rgb(" + r + "," + g + "," + b + ")";
}

//Send colors to ESP32
function sendColor() {
  var r = document.getElementById("rSlider").value;
  var g = document.getElementById("gSlider").value;
  var b = document.getElementById("bSlider").value;

  // Send a GET request with args
  fetch("/?r=" + r + "&g=" + g + "&b=" + b).then(function () {
    console.log("Color sent: R=" + r + " G=" + g + " B=" + b);
  });
}

//Send a GET request with args to reset colors to 0
function resetColor() {
  fetch("/?r=" + 0 + "&g=" + 0 + "&b=" + 0).then(function () {
    console.log("Color sent: R=" + 0 + " G=" + 0 + " B=" + 0);
  });
  var r = 0;
  var g = 0;
  var b = 0;

  document.getElementById("rVal").textContent = r;
  document.getElementById("gVal").textContent = g;
  document.getElementById("bVal").textContent = b;

  document.getElementById("rSlider").value = r;
  document.getElementById("gSlider").value = g;
  document.getElementById("bSlider").value = b;

  document.getElementById("preview").style.background =
    "rgb(" + r + "," + g + "," + b + ")";
}

// Initialize the page with the correct preview color and labels
update();
