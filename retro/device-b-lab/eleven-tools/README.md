# eleven.mjs

Zero-dependency Node (>= 18) client for ElevenLabs, house style of tools/meshy/meshy.mjs. Key `ELEVEN_LABS_API_KEY` from the environment or from `C:/Users/kazda/jam/thirdjam/.env` (override with `ELEVEN_ENV_FILE`); it is only sent in the `xi-api-key` header.

    node eleven.mjs quota
    node eleven.mjs voices --category premade --search "old"
    node eleven.mjs tts --voice <id> --text "Action!" --out a.mp3 [--stability .4 --style .5]
    node eleven.mjs sfx --prompt "..." --seconds 1 [--loop] [--influence .5] --out s.mp3   (API minimum 0.5 s)
    node eleven.mjs music --prompt "..." --seconds 45 [--instrumental false] --out m.mp3
    common: --dry-run  --ledger file.jsonl (or ELEVEN_LEDGER)  --format mp3_44100_128

Last stdout line is one JSON object `{"tool":"thirdjam.eleven/1","cmd":...,"ok":...}`. Exit 0 ok, 1 API error, 2 could not run. POSTs (which cost credits) are never retried; GETs retry twice.
`batch1.mjs` (voice and SFX batch), `post.mjs` (ffmpeg trim + measurements) and `page.mjs` (listen.html) are the one-off runners for lane AU1.
