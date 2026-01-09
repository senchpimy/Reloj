import os
import spotipy
from spotipy.oauth2 import SpotifyOAuth
from spotipy.exceptions import SpotifyException
from dotenv import load_dotenv

load_dotenv()


class SpotifyClient:
    def __init__(self) -> None:
        # NOTA: Se agregó 'user-modify-playback-state' para poder controlar la música
        scope_list = (
            "user-read-currently-playing "
            "user-read-playback-state "
            "user-modify-playback-state"
        )

        sp = spotipy.Spotify(
            auth_manager=SpotifyOAuth(
                client_id=os.getenv("SPOTIPY_CLIENT_ID"),
                client_secret=os.getenv("SPOTIPY_CLIENT_SECRET"),
                redirect_uri="http://127.0.0.1:8888/callback",
                scope=scope_list,
            )
        )
        self.sp = sp

    def get_playback_info(self):
        playback = self.sp.current_playback()

        result = {"is_playing": False, "title": None,
                  "artist": None, "cover_url": None}

        if playback and playback.get("is_playing"):
            result["is_playing"] = True
            content_type = playback.get("currently_playing_type")
            track = playback.get("item")

            if track:
                if content_type == "track":
                    result["title"] = track.get("name")
                    artists = [artist["name"]
                               for artist in track.get("artists", [])]
                    result["artist"] = ", ".join(artists) if artists else None
                    if track.get("album") and track["album"].get("images"):
                        result["cover_url"] = track["album"]["images"][0]["url"]
                elif content_type == "episode":
                    result["title"] = track.get("name")
                    result["artist"] = track.get("show", {}).get("publisher")
                    if track.get("show") and track["show"].get("images"):
                        result["cover_url"] = track["show"]["images"][0]["url"]
            else:
                result["cover_url"] = None

        return result

    def play(self, context_uri=None):
        """Reanuda la reproducción o reproduce una canción/lista específica si se da uri."""
        try:
            if context_uri:
                self.sp.start_playback(context_uri=context_uri)
            else:
                self.sp.start_playback()
            print("Reproduciendo...")
        except SpotifyException as e:
            print(f"Error al reproducir: {e}")

    def pause(self):
        """Pausa la reproducción actual."""
        try:
            self.sp.pause_playback()
            print("Pausado.")
        except SpotifyException as e:
            print(f"Error al pausar (quizás ya está pausado): {e}")

    def next_track(self):
        """Salta a la siguiente canción."""
        try:
            self.sp.next_track()
            print("Siguiente canción.")
        except SpotifyException as e:
            print(f"Error al saltar canción: {e}")

    def previous_track(self):
        """Vuelve a la canción anterior."""
        try:
            self.sp.previous_track()
            print("Canción anterior.")
        except SpotifyException as e:
            print(f"Error al retroceder canción: {e}")

    def set_volume(self, volume: int):
        """Ajusta el volumen (0 a 100)."""
        try:
            # Asegurar que el volumen esté entre 0 y 100
            volume = max(0, min(100, volume))
            self.sp.volume(volume)
            print(f"Volumen ajustado a {volume}%")
        except SpotifyException as e:
            print(
                f"Error al ajustar volumen (algunos dispositivos no lo soportan): {e}")


# Ejemplo de uso (descomentar para probar):
if __name__ == "__main__":
    client = SpotifyClient()
    info = client.get_playback_info()
    print(f"Info actual: {info}")

    # client.next_track()
    client.pause()
