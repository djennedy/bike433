<script setup lang="ts">

import "leaflet/dist/leaflet.css";
import {LCircleMarker, LMap, LPolyline, LPopup, LTileLayer} from "@vue-leaflet/vue-leaflet";
import {onMounted, ref} from "vue";
import { io } from "socket.io-client";

const UDP_URL = "http://127.0.0.1:12345";

import { useToast } from "vue-toastification";
const toast = useToast();

const socket = io();
socket.on("connect", () => {
    console.log(`Connected to server ${UDP_URL}`);
});


const isMoved = ref(false);
const isLocked = ref(false);
const latestLat = ref(0);
const latestLong = ref(0);
const coordinates = ref<Number[][]>([]);
const images = ref<{image_url:string, coordinate:Number[]}[]>([]);
const stopped = ref(false);


const getLat = async function(){
    socket.emit("udpCommand","get_lat",(response : string)=>{
        handleInvalidCases(response);
        latestLat.value = Number.parseFloat(response);
    })
}

const getLong = async function (){
    socket.emit("udpCommand","get_long",(response : string)=>{
        handleInvalidCases(response);
        latestLong.value = Number.parseFloat(response);
    })
}

const updateCoordinate = function (){
    getLat()
        .then(() => getLong())
        .then(() =>{
            let coordinate = [latestLat.value, latestLong.value];
            coordinates.value.push(coordinate);
        });
}

const getIsLocked = function(){
    socket.emit("udpCommand","get_lock", (response : string) =>{
        isLocked.value = response.includes("is locked");
    })
}

const setLock = function(){
    socket.emit("udpCommand","lock",(response : string) =>{
        console.log(`Lock status:` + response);
    })
}

const setUnlock = function(){
    socket.emit("udpCommand","unlock",(response : string)=>{
        console.log(`Lock status: ` + response);
    })
}

const getIsMoved = function(){
    socket.emit("udpCommand","get_isMoved",(response : string) =>{
        isMoved.value = response.includes("is moved");
    })
}

const getImages = function(){
    socket.emit("webcam-capture", (response : string)=>{
        images.value.push({
            image_url: response,
            coordinate: [latestLat.value, latestLong.value]
        });
    })
}

const  updateInfo = function (){
    setInterval(function(){
        updateCoordinate();
        getIsLocked();
        getIsMoved();
        getImages();
    },5000);
}

const toggleLock = function(){
    isLocked.value = ! isLocked.value;
    if(isLocked.value){
        setLock();
    }else{
        setUnlock();
    }
}
const stopTracking = function(){
    socket.emit("udpCommand","stop",(response : string) =>{
        console.log(response);
        stopped.value = true;
    })
}

const handleInvalidCases = function (this: any, response : string){
    if(response.includes("invalid")){
        toast.warning("Unable to get GPS signal. Please be patience as this is not the best GPS unit :(");
    }
}

const zoom = ref(18);
const surreyCoordinate = ref([49.1891913,-122.850232]);

onMounted(()=>{
    updateInfo();
})

</script>

<template>

    <main class="flex flex-col items-center gap-4">
        <h3 class="w-screen text-3xl p-4">Anti Bike-theft Device</h3>
        <p class=" rounded w-1/2 h-20 bg-green-400 text-center flex items-center justify-center text-4xl text-white" :class="{'bg-red-500':isMoved && isLocked}" >
            {{ isMoved && isLocked ? 'Stolen!!!' : 'Not Stolen!' }}
        </p>
        <div class="flex flex-row gap-4 items-center ">
            <button @click="toggleLock" class="p-4 rounded" :class="isLocked ? `bg-red-500` : `bg-green-500`">{{isLocked ? `Locked` : `Unocked`}}</button>
            <button @click="stopTracking" class="p-4 rounded" :disabled="stopped" :class="stopped ? `bg-gray-500` : `bg-red-500`">
                {{stopped ? `Tracking stopped!` : `Stop Tracking`}}
            </button>
        </div>
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
                    <template v-for="{image_url, coordinate} in images" v-bind:key="image_url">
                        <l-circle-marker :radius="3" :lat-lng="coordinate" >
                            <l-popup>
                                <div>
                                    {{`Coordinate: ${coordinate}`}}
                                    <img :src="image_url" alt="Footage from webcam">
                                </div>
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
