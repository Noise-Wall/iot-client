<script setup>
import axios from "axios";
import { ref } from "vue";

// const data = async () => await get();

const tempC = ref(0);
const tempF = ref(0);
const humidity = ref(0);

const isOn = ref(true);

function api() {
  try {
    return axios.create({
      baseURL: "http://192.168.1.199",
      headers: {
        Accept: "application/json",
        "Content-Type": "application/json",
      },
      // withCredentials: true,
    });
  } catch (error) {
    console.log(error.message);
    return error;
  }
}

async function get() {
  return api()
    .get("/temperature")
    .catch(function errCatch(err) {
      console.log(err.message);
    });
}

async function post() {
  const json = isOn.value ? { status: off } : { status: on };
  return api()
    .post("/led", json)
    .then((res) => {
      console.log(res)
    })
    .catch(function errCatch(err) {
      console.log(err.message);
    });
}

async function updateData() {
  const newData = await get();
  tempC.value = newData.data.tempC;
  tempF.value = newData.data.tempF;
  humidity.value = newData.data.tempH;
  console.log(newData.data);
}

setInterval(updateData, 1000);
</script>

<template>
  <div class="container">
    <h1>Cliente IoT</h1>
    <p>
      Este cliente recibe informacion de una tarjeta de WiFi ESP8266 con un
      sensor DHT 11 conectado a ella.
    </p>
    <p>
      La informacion se recaba mediante peticiones HTTP con Axios, encubiertas
      en una funcion que corre en un intervalo de 1 segundo, y las respuestas
      empaquetadas en formato JSON.
    </p>
    <p>
      No es muy eficiente como los websockets o MQTT, pero es lo que pude lograr
      hacer funcionar.
    </p>
    <p>
      Si se actualiza pero las variaciones son bajas. Checar la consola para ver
      las respuestas de las peticiones.
    </p>
    <div class="labels" v-if="tempC > 0 || tempF > 0 || humidity > 0">
      <label for="celsius">Celsius: {{ tempC.trim() }} C°</label>
      <label for="fahrenheit">Fahrenheit°: {{ tempF.trim() }} F°</label>
      <label for="humidity">Humidity: {{ humidity.trim() }}</label>
      <button @click="post"> {{ isOn ? 'Apagar sensor' : 'Prender sensor'}}</button>
    </div>
    <h3 v-else>
      Hubo un error de conexion.
    </div>
  </div>
</template>

<style scoped>
* {
  box-sizing: border-box;
  font-family: "Segoe UI", Tahoma, Geneva, Verdana, sans-serif;
  color: #6c6c6c;
  font-weight: 400;
  margin: 15px;
}

body {
  background-color: #da6a6a;
}

.container {
  display: flex;
  flex-direction: column;
}

.labels {
  border: 1px solid #3c3c3c;
  border-radius: 5px;
  width: 50%;
  min-height: 10vh;
  display: flex;
  flex-direction: column;
  justify-content: space-between;
}

p {
  width: 70%;
}
</style>
