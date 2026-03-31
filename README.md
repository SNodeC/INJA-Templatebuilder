# INJA-Templatebuilder (SNode.C + INJA C++)

Interactive INJA template builder with:

- **Backend:** SNode.C `express`-style web API in C++
- **Template engine:** INJA (C++)
- **Frontend:** static HTML/CSS/JS playground
- **Telemetry feedback:** render/validation metadata included in REST JSON responses

## Project layout

- `backend/src/main.cpp` — REST backend using SNode.C
- `public/` — frontend UI
- `docs/snodec-mqttsuite-review.md` — extended review of SNode.C and mqttsuite
- `CMakeLists.txt` — build definition

## REST API

- `GET /api/health`
- `GET /api/examples`
- `POST /api/validate`
- `POST /api/render`

## Build (example)

```bash
cmake -S . -B build
cmake --build build
./build/inja-templatebuilder
```

Then open `http://localhost:3000`.

## Notes

- Backend is now implemented in **SNode.C**, not Node.js.
- Telemetry is returned directly in JSON response bodies (`meta`) for render/validation requests.
