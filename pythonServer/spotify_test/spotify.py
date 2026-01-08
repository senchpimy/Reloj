import os
import spotipy
from spotipy.oauth2 import SpotifyOAuth
import webbrowser
import json  # Para depurar
from dotenv import load_dotenv

load_dotenv()

class SpotifyClient:
    def __init__(self) -> None:
        sp = spotipy.Spotify(
            auth_manager=SpotifyOAuth(
                client_id=os.getenv("SPOTIPY_CLIENT_ID"),
                client_secret=os.getenv("SPOTIPY_CLIENT_SECRET"),
                redirect_uri="http://localhost:8888/callback",
                scope="user-read-currently-playing user-read-playback-state",
            )
        )
        self.sp = sp

    def get_playback_info(self):
        playback = sp.current_playback()

        result = {"is_playing": False, "title": None, "artist": None, "cover_url": None}

        if playback and playback.get("is_playing"):
            result["is_playing"] = True
            content_type = playback.get("currently_playing_type")
            track = playback.get("item")

            if track:
                if content_type == "track":
                    result["title"] = track.get("name")
                    artists = [artist["name"] for artist in track.get("artists", [])]
                    result["artist"] = ", ".join(artists) if artists else None
                    if track.get("album") and track["album"].get("images"):
                        result["cover_url"] = track["album"]["images"][0]["url"]
                elif content_type == "episode":
                    result["title"] = track.get("name")
                    result["artist"] = track.get("show", {}).get("publisher")
                    if track.get("show") and track["show"].get("images"):
                        result["cover_url"] = track["show"]["images"][0]["url"]
            else:
                result["cover_url"] = "https://example.com/default_cover.png"

        return result
