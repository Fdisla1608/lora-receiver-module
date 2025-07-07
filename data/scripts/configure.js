const url = window.location;
const params = new URLSearchParams(url.search);

const ssid = params.get("ssid");
const password = params.get("password");

function setLoadingScreen(show) {
  const loadingScreen = document.querySelector(".loading-screen");
  loadingScreen.style.display = show ? "flex" : "none";
}

async function Configure() {
  try {
    console.log("Preparing to configure...");
    const txtBoardType = document.getElementById("board-type");
    setLoadingScreen(true);

    const response = await fetch("/configure", {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify({
        boardType: txtBoardType.value,
      }),
    });

    if (!response.ok) {
      throw new Error("Network response was not ok");
    }

    const payload = await response.json();

    if (payload.status === "ok") {
      window.location.replace("/success");
    } else {
      window.location.replace("/pending");
    }
  } catch (error) {
    console.error("Error al configurar:", error);
    alert(
      "Hubo un error al configurar. Verifica los datos e intenta nuevamente."
    );
  } finally {
    setLoadingScreen(false);
  }
}

window.onload = () => {
  setLoadingScreen(false);
};
