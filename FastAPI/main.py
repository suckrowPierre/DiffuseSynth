from fastapi import FastAPI, HTTPException
from diffusers import AudioLDMPipeline
from diffusers import AudioLDM2Pipeline
from pydantic import BaseModel
import torch
import base64
import io
import soundfile as sf
from enum import Enum
import numpy as np
import matplotlib.pyplot as plt
import librosa
import librosa.display
from typing import Optional


app = FastAPI()
pipe = None
samplerate = 16000

class Models(Enum):
    # S = "audioldm-s-full"
    ALDM_SV2 = "audioldm-s-full-v2"
    ALDM_M = "audioldm-m-full"
    ALDM_L = "audioldm-l-full"
    ALDM_2 = "audioldm2"
    ALDM_2_L = "audioldm2-large"
    ALDM_2_MUSIC = "audioldm2-music"


class Devices(Enum):
    CPU = "cpu"
    CUDA = "cuda"
    MPS = "mps"



def get_models_names():
    return [model.value for model in Models]


def get_devices_names():
    return [device.value for device in Devices]


def is_repo_id_valid(repo_id):
    return repo_id in Models._value2member_map_


def is_device_valid(device):
    return device in Devices._value2member_map_

def plot_melspectrogram(waveform, sr, n_fft=2048, hop_length=512, n_mels=128):
    plt.figure(figsize=(11, 8))
    S = librosa.feature.melspectrogram(y=waveform, sr=sr, n_fft=n_fft, hop_length=hop_length, n_mels=n_mels)
    S_DB = librosa.amplitude_to_db(S, ref=np.max)
    librosa.display.specshow(S_DB, sr=sr, hop_length=hop_length, x_axis='time', y_axis='mel')
    plt.colorbar(format='%+2.0f dB')
    plt.tight_layout()
    plt.savefig('test.png', dpi=300)

class GenerateParams(BaseModel):
    prompt: str
    negative_prompt: str
    audio_length_in_s: float
    num_inference_steps: int
    guidance_scale: float
    seed: Optional[int] = None


class SetupParams(BaseModel):
    device: str
    repo_id: str


class AudioModel:
    def __init__(self, params: SetupParams):
        self.device = params.device
        self.repo_id = "cvssp/" + params.repo_id
        print(self.repo_id)
        self.dtype = torch.float32 if self.device != "cuda" else torch.float16
        self.pipe = self.setup_pipe()

    def setup_pipe(self):
        if self.repo_id == "cvssp/audioldm2" or self.repo_id == "cvssp/audioldm2-large" or self.repo_id == "cvssp/audioldm2-music":
            pipe = AudioLDM2Pipeline.from_pretrained(self.repo_id, torch_dtype=self.dtype)
        else:
            pipe = AudioLDMPipeline.from_pretrained(self.repo_id, torch_dtype=self.dtype)

        pipe = pipe.to(self.device)
        if self.device == "mps":
            pipe.enable_attention_slicing()
        return pipe

    def generate(self, params: GenerateParams):
        print("here2")
        if params.seed is not None:
            generator = torch.Generator(self.device).manual_seed(params.seed)
            return self.pipe(prompt=params.prompt,
                             audio_length_in_s=params.audio_length_in_s,
                             num_inference_steps=params.num_inference_steps,
                             guidance_scale=params.guidance_scale,
                             negative_prompt=params.negative_prompt,
                             generator=generator
                             ).audios[0]
        else:
            return self.pipe(prompt=params.prompt,
                         audio_length_in_s=params.audio_length_in_s,
                         num_inference_steps=params.num_inference_steps,
                         guidance_scale=params.guidance_scale,
                         negative_prompt=params.negative_prompt).audios[0]


audio_model = None


@app.get("/")
async def root():
    return {
        "message": "alive"
    }



@app.get("/model-status")
async def get_model_status():
    return {
        "isModelSetup": audio_model is not None
    }

@app.get("/setup_parameters")
async def get_setup_parameters():
    return {
        "devices": get_devices_names(),
        "models": get_models_names(),
    }

@app.post("/setup")
async def setup(params: SetupParams):
    if not is_repo_id_valid(params.repo_id):
        raise HTTPException(status_code=400, detail="Model not found. Please choose one of: " + str(get_models_names()))
    if not is_device_valid(params.device):
        raise HTTPException(status_code=400,
                            detail="Device not found. Please choose one of: " + str(get_devices_names()))
    global audio_model
    audio_model = AudioModel(params)
    return {"message": "Setting up finished"}

@app.get("/current_parameters")
async def get_current_model_and_device():
    global audio_model
    if audio_model is None:
        raise HTTPException(status_code=400, detail="Model is not set up. Please POST to /setup first.")
    return {"device": audio_model.device, "model": audio_model.repo_id}


@app.post("/generate")
async def generate(params: GenerateParams):
    print("here0")
    global audio_model
    if audio_model is None:
        raise HTTPException(status_code=400, detail="Model is not set up. Please POST to /setup first.")
    audio = audio_model.generate(params)
    plot_melspectrogram(audio, samplerate)

    with io.BytesIO() as audio_io:
        sf.write("output.wav", audio, samplerate, format='WAV')
        sf.write(audio_io, audio, samplerate, format='WAV')
        audio_bytes = audio_io.getvalue()


    audio_base64 = base64.b64encode(audio_bytes).decode('utf-8')

    return {"audio": audio_base64 , "message": "Audio generated successfull with prompt: " + params.prompt}
