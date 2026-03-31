const exampleSelect = document.querySelector('#example');
const templateEl = document.querySelector('#template');
const dataEl = document.querySelector('#data');
const outputEl = document.querySelector('#output');
const eventsEl = document.querySelector('#events');

function pushEvent(label, payload) {
  const li = document.createElement('li');
  li.textContent = `${new Date().toLocaleTimeString()} ${label}: ${JSON.stringify(payload)}`;
  eventsEl.prepend(li);

  while (eventsEl.children.length > 40) {
    eventsEl.removeChild(eventsEl.lastChild);
  }
}

async function loadExamples() {
  const res = await fetch('/api/examples');
  const examples = await res.json();

  examples.forEach((entry, idx) => {
    const opt = document.createElement('option');
    opt.value = idx;
    opt.textContent = entry.name;
    exampleSelect.appendChild(opt);
  });

  const applyExample = () => {
    const current = examples[Number(exampleSelect.value)] ?? examples[0];
    templateEl.value = current.template;
    dataEl.value = JSON.stringify(current.data, null, 2);
  };

  exampleSelect.addEventListener('change', applyExample);
  applyExample();
}

async function callApi(path) {
  const body = {
    template: templateEl.value,
    data: dataEl.value
  };

  const res = await fetch(path, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(body)
  });

  return res.json();
}

document.querySelector('#renderBtn').addEventListener('click', async () => {
  const payload = await callApi('/api/render');

  if (payload.ok) {
    outputEl.textContent = payload.output;
    pushEvent('render-local-ok', payload.meta);
  } else {
    outputEl.textContent = payload.error;
    pushEvent('render-local-error', payload.meta ?? { error: payload.error });
  }
});

document.querySelector('#validateBtn').addEventListener('click', async () => {
  const payload = await callApi('/api/validate');

  if (payload.ok && payload.valid) {
    pushEvent('validate-ok', payload);
  } else {
    pushEvent('validate-error', payload);
  }
});

loadExamples();
