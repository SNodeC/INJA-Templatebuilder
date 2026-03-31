# INJA-Templatebuilder (SNode.C + INJA C++)

Interactive INJA template builder with:

- **Backend:** SNode.C `express`-style web API in C++
- **Template engine:** INJA (C++)
- **Frontend:** static HTML/CSS/JS playground
- **Realtime feedback:** SSE endpoint for event updates

## Project layout

- `backend/src/main.cpp` — REST + SSE backend using SNode.C
- `public/` — frontend UI
- `docs/snodec-mqttsuite-review.md` — extended review of SNode.C and mqttsuite
- `CMakeLists.txt` — build definition

## REST/SSE API

- `GET /api/health`
- `GET /api/examples`
- `POST /api/validate`
- `POST /api/render`
- `GET /api/events` *(SSE stream frames)*

## Build (example)

```bash
cmake -S . -B build
cmake --build build
./build/inja-templatebuilder
```

Then open `http://localhost:3000`.

## Notes

- Backend is now implemented in **SNode.C**, not Node.js.
- SSE is used to deliver connection and render telemetry events to the frontend.
