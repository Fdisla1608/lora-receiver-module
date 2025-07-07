const loadingScreen = document.querySelector(".loading-screen");
const source = new EventSource("/events");

function setLoading(show) {
  loadingScreen.style.display = show ? "flex" : "none";
}

source.addEventListener("status", (event) => {
  const data = JSON.parse(event.data);
  console.log("Status:", data);

  setLoading(false);

  if (data.status === "GOT_IP") {
    window.location.replace("/boards");
  }
});

source.addEventListener("scan", (event) => {
  const wifiSelect = document.getElementById("ssid-text");
  const networks = JSON.parse(event.data);
  while (wifiSelect.length > 0) {
    wifiSelect.remove(0);
  }

  networks.forEach((net) => {
    const option = document.createElement("option");
    option.value = net.ssid;
    option.textContent = net.ssid;
    wifiSelect.appendChild(option);
  });

  setLoading(false);
});

async function connectNetwork() {
  try {
    const ssid = document.getElementById("ssid-text").value;
    const password = document.getElementById("password-text").value;
    const advanced = document.getElementById("advance-checkbox").checked;

    const data = {
      typeConnection: "WiFi",
      ssid,
      password,
    };

    if (advanced) {
      data.ip = document.getElementById("ip-text").value;
      data.mask = document.getElementById("subnet-text").value;
      data.gtw = document.getElementById("gateway-text").value;
    }

    setLoading(true);

    const response = await fetch("/connect", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(data),
    });

    if (!response.ok) throw new Error("Error al conectar");

    const result = await response.text();
    console.log("Response:", result);
  } catch (error) {
    alert("¡No se pudo conectar a la red!");
    console.error(error);
  }
}

function showPassword() {
  const txtPass = document.getElementById("password-text");
  const txtPassButton = document.getElementById("show-pwd-button");
  txtPass.type = txtPass.type === "password" ? "text" : "password";
  txtPassButton.textContent = txtPass.type === "password" ? "👁️" : "🔒";
}

function toggleAdvancedInputs() {
  const enabled = document.getElementById("advance-checkbox").checked;
  document
    .querySelectorAll(".red-avanzado-section-container input")
    .forEach((input) => {
      input.disabled = !enabled;
    });
}

async function ScanWifi() {
  try {
    setLoading(true);
    const response = await fetch("/scan");
    await response.json();
  } catch (err) {
    alert("Error al escanear redes WiFi.");
    setLoading(false);
  }
}

window.onload = () => {
  toggleAdvancedInputs();
  setLoading(false);
};
