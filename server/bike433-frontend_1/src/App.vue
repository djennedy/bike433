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
const stopped = ref(false);
const refreshes = ref(0);
const invalidText = "invalid";
let isInvalid = true;
const image = ref("");
const imageRefresh = ref(0);
let openStreetMapURL = ref("https://www.openstreetmap.org/");


const getLat = async function(isClicked? : Boolean){
    socket.emit("udpCommand","get_lat",(response : string)=>{
        latestLat.value = Number.parseFloat(response);
        if (isNaN(latestLat.value)) {
            isInvalid = true;
        } else {
            isInvalid = false;
        }
    })
}

const getLatButton = async () => {
        socket.emit("udpCommand","get_lat",(response : string)=>{
            handleInvalidCases(response);
            latestLat.value = Number.parseFloat(response);
    })
}

const getLong = async function (isClicked? : Boolean){
    socket.emit("udpCommand","get_long",(response : string)=>{
        latestLong.value = Number.parseFloat(response);
        if (isNaN(latestLong.value)) {
            isInvalid = true;
        } else {
            isInvalid = false;
        }
    })
}

const getLongButton = async function (){
    socket.emit("udpCommand","get_long",(response : string)=>{
        handleInvalidCases(response);
        latestLong.value = Number.parseFloat(response);
    })
}

const updateCoordinate = function (){
    getLat(false)
        .then(() => getLong(false))
        .then(() =>{
            if(isNaN(latestLong.value) || isNaN(latestLat.value)){
                return;
            }
            let coordinate = [latestLat.value, latestLong.value];
            console.log(coordinate);
            coordinates.value.push(coordinate);
            console.log(coordinates);
            if (coordinates.value.length > 1024) {
                coordinates.value.shift();
            }
        })
        .then(() =>  {
            openStreetMapURL.value = `https://www.openstreetmap.org/search?query=${latestLat.value}%20${latestLong.value}`
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
        image.value = response;
        imageRefresh.value += 1;
    })
}

const  updateInfo = function (){
    setInterval(function(){
        console.log(`updated`);
        updateCoordinate();
        getIsLocked();
        getIsMoved();
        refreshes.value += 1;
    },500);
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
        isInvalid = true;
    }
    if(isNaN(Number.parseFloat(response))){
        return;
    }
}

const zoom = ref(18);
const surreyCoordinate = ref([49.1891913,-122.850232]);

onMounted(()=>{
    updateInfo();
    setInterval(getImages,5000);
})

</script>

<template>

    <main class="flex flex-col items-center gap-4">
        <h3 class="w-screen text-3xl p-4">Anti Bike-theft Device</h3>
        <p class=" rounded w-1/2 h-20 bg-green-400 text-center flex items-center justify-center text-4xl text-white" 
        :class="{'bg-red-500':isMoved && isLocked}"
        :key="refreshes" >
            {{ isMoved && isLocked ? 'Stolen!!!' : 'Not Stolen!' }}
        </p>
        <div class="flex flex-col gap-4 items-center ">
            <div class="flex-row gap-x-4">
                <button @click="toggleLock" class="p-4 rounded" :class="isLocked ? `bg-red-500` : `bg-green-500`" :key="refreshes">{{isLocked ? `Locked` : `Unlocked`}}</button>
                <button @click="stopTracking" class="p-4 rounded" :disabled="stopped" :class="stopped ? `bg-gray-500` : `bg-red-500`" :key="refreshes">
                    {{stopped ? `Tracking stopped!` : `Stop Tracking`}}
                </button>
            </div>
            <p v-if="isInvalid">
                Latitude Invalid! Please use the device outdoors or wait the GPS to get a fix.
            </p>
            <p v-else>
                Current Latitude: {{ latestLat }}
            </p>
            <p v-if="isInvalid">
                Longitude Invalid! Please use the device outdoors or wait the GPS to get a fix.
            </p>
            <p v-else>
                Current Longitude: {{ latestLong }}
            </p>

            <a :href="openStreetMapURL">
                <button class="p-4 rounded bg-blue-500">
                    Open location in Open Street Map!
                </button>
            </a>
            <img style="width: 1280px; height: 640px" :src="image" alt="Image footage" :key="imageRefresh"/>

        </div>
        <div class="flex">
        </div>
        
    </main>
</template>

<style scoped>
</style>
