<script setup lang="ts">

import "leaflet/dist/leaflet.css";
import {LCircleMarker, LMap, LPolyline, LPopup, LTileLayer} from "@vue-leaflet/vue-leaflet";
import {onMounted, ref} from "vue";
import { io } from "socket.io-client";
const UDP_URL = "http://127.0.0.1:12345";
const WEBCAM_URL = "http://110.114.160.9:8080/live.mpd";
const socket = io();
socket.on("connect", () => {
    console.log(`Connected to server ${UDP_URL}`);
});


const isMoved = ref(false);
const isLocked = ref(false);
const latestLat = ref(0);
const latestLong = ref(0);
const coordinates = ref<Number[][]>([]);
const images = ref<{a:string, b:Number[]}[]>([]);


const getLat = async function(){
    socket.emit("get_lat",(response : string)=>{
        latestLat.value = Number.parseFloat(response);
    })
}

const getLong = async function (){
    socket.emit("get_long",(response : string)=>{
        latestLong.value = Number.parseFloat(response);
    })
}

const updateCoordinate = function (){
    getLat()
        .then(() => getLong())
        .then(() =>{
            let coordinate = [latestLat.value, latestLong.value];
            coordinates.value.push(coordinate);
        })
}

const getIsLocked = function(){
    socket.emit("get_lock", (response : string) =>{
        isLocked.value = response.includes("is locked");
    })
}

const setLock = function(){
    socket.emit("lock",(response : string) =>{
        console.log(`Lock status:` + response);
    })
}

const setUnlock = function(){
    socket.emit("unlock",(response : string)=>{
        console.log(`Lock status: ` + response);
    })
}

const getIsMoved = function(){
    socket.emit("get_isMoved",(response : string) =>{
        isMoved.value = response.includes("is moved");
    })
}

const getImages = function(){
    socket.emit("webcam-capture", (response : string)=>{

    })
}

const  updateInfo = function (){
    setInterval(function(){
        updateCoordinate();
        getIsLocked();
        getIsMoved();
    },5000);
}

// const initPlayer = function(){
//     player.initialize(document.querySelector("#camera-video") as HTMLElement, url, true);
//     player.on(dashjs.MediaPlayer.events.ERROR, (e) => {
//         if (e.error === "download") {
//             console.log("Unable to get footage from camera");
//         }
//     });
// }
const zoom = ref(18);
const surreyCoordinate = ref([49.1891913,-122.850232]);

onMounted(()=>{
    updateInfo();
})

</script>

<template>

    <main class="flex flex-col items-center gap-4">
        <h3 class="w-screen text-3xl p-4">Anti Bike-theft Device</h3>
        <p class="w-1/2 h-20 bg-green-400 text-center flex items-center justify-center text-4xl text-white" :class="{'bg-red-500':isMoved && isLocked}" >
            {{ isMoved && isLocked ? 'Stolen!!!' : 'Not Stolen!' }}
        </p>

        <div class="flex flex-row gap-4 w-1/3 h-1/3">
            <!--          <video id="camera-video" width=600 height=300 class="video-js vjs-default-skin" controls></video>-->
            <div style="height:600px; width:800px">
                <l-map ref="map" v-model:zoom="zoom" :center="surreyCoordinate" :use-global-leaflet="false">
                    <l-tile-layer
                        url="https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png"
                        layer-type="base"
                        name="OpenStreetMap"
                    ></l-tile-layer>
                    <l-polyline :lat-lngs="coordinates"></l-polyline>
                    <template v-for="coordinate in coordinates" v-bind:key="coordinate">
                        <l-circle-marker :radius="3" :lat-lng="coordinate" >
                            <l-popup>
                                {{`Coordinate: ${coordinate}`}}
                            </l-popup>
                        </l-circle-marker>
                    </template>
                </l-map>
            </div>
        </div>
    </main>
</template>

<style scoped>
</style>
