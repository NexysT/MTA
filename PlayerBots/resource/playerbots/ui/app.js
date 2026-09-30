"use strict";

const $ = id => document.getElementById(id);
const send = (name, payload) => {
  if (window.mta && typeof mta.triggerEvent === "function") {
    mta.triggerEvent(name, ...(payload === undefined ? [] : [payload]));
  } else {
    playerBotsNotice("Preview outside MTA.", true);
  }
};
const esc = s => String(s ?? "").replace(/[&<>"']/g, c => ({"&":"&amp;","<":"&lt;",">":"&gt;","\"":"&quot;","'":"&#39;"}[c]));

let databaseNames = [];
let selectedSpecific = new Set();

const nameModes = ["random", "specific", "custom"];
const nameModeLabels = {
  random: "Base de dados de nomes aleatórios",
  specific: "Nomes específicos da lista",
  custom: "Nomes personalizados"
};

const closeNameModeMenu = () => {
  $("nameModeMenu").classList.add("hidden");
  $("nameModeButton").setAttribute("aria-expanded", "false");
};

const setNameMode = (value, runUpdate = true) => {
  const mode = nameModes.includes(value) ? value : "custom";
  $("nameMode").value = mode;
  $("nameModeLabel").textContent = nameModeLabels[mode];
  $("nameModeMenu").querySelectorAll(".modeSelectOption").forEach(option => {
    const active = option.dataset.value === mode;
    option.classList.toggle("selected", active);
    option.setAttribute("aria-selected", active ? "true" : "false");
  });
  closeNameModeMenu();
  if (runUpdate) updateConditionals();
};

const stepNameMode = direction => {
  const current = Math.max(0, nameModes.indexOf($("nameMode").value));
  const next = (current + direction + nameModes.length) % nameModes.length;
  setNameMode(nameModes[next]);
};

window.playerBotsNotice = (text, isError) => {
  const e = $("notice");
  e.textContent = String(text);
  e.className = "notice " + (isError ? "error" : "ok");
};

const requiredNames = () => {
  const target = Number($("target").value) || 0;
  if (!$("replaceEnabled").checked) return target;
  return Math.min(target, Number($("replaceAt").value) || target);
};

const updateSpecificCount = () => {
  const selected = selectedSpecific.size;
  const needed = requiredNames();
  $("specificCount").textContent = `${selected} selecionado${selected === 1 ? "" : "s"}`;
  $("specificHint").textContent = selected < needed
    ? `Selecionaste ${selected}; para esta configuração precisas de pelo menos ${needed}.`
    : `${selected} nomes disponíveis para até ${needed} bots.`;
  $("specificHint").classList.toggle("warning", selected < needed);
};

const renderSpecificNames = () => {
  const q = ($("specificSearch").value || "").trim().toLocaleLowerCase("pt-PT");
  const visible = databaseNames.filter(name => !q || name.toLocaleLowerCase("pt-PT").includes(q));
  $("specificNames").innerHTML = visible.length ? visible.map(name => {
    const checked = selectedSpecific.has(name) ? " checked" : "";
    return `<label class="nameChoice"><input type="checkbox" data-name="${esc(name)}"${checked}><span>${esc(name)}</span></label>`;
  }).join("") : '<div class="empty compact">A base está vazia ou não há resultados. Edita names.lua.</div>';

  $("specificNames").querySelectorAll('input[type="checkbox"]').forEach(input => {
    input.addEventListener("change", () => {
      const name = input.dataset.name;
      if (input.checked) selectedSpecific.add(name); else selectedSpecific.delete(name);
      updateSpecificCount();
    });
  });
  updateSpecificCount();
};

const updateConditionals = () => {
  const mode = $("nameMode").value;
  $("customWrap").classList.toggle("hidden", mode !== "custom");
  $("specificWrap").classList.toggle("hidden", mode !== "specific");
  $("databaseHint").classList.toggle("hidden", mode !== "random");
  $("replaceWrap").classList.toggle("hidden", !$("replaceEnabled").checked);
  if (mode === "specific") renderSpecificNames();
};

window.playerBotsReceive = raw => {
  const d = Array.isArray(raw) ? raw[0] : raw;
  if (!d || typeof d !== "object") return;

  const bots = Array.isArray(d.bots) ? d.bots : [];
  const settings = d.settings || {};
  const max = Number(d.max) || 32;

  databaseNames = Array.isArray(d.databaseNames) ? d.databaseNames.map(String) : [];
  selectedSpecific = new Set(Array.isArray(settings.specificNames) ? settings.specificNames.map(String) : []);

  $("real").textContent = d.real ?? 0;
  $("bots").textContent = bots.length;
  $("total").textContent = d.public ?? ((Number(d.real) || 0) + bots.length);
  $("max").textContent = max;
  $("moduleCount").textContent = bots.length;
  $("configured").textContent = settings.target ?? 0;
  $("publicCount").textContent = `${d.public ?? 0}/${max}`;
  $("native").textContent = d.nativeReady ? "Ligado" : "Indisponível";

  const pill = $("status");
  pill.textContent = d.enabled ? "Ativo" : "Desativado";
  pill.className = "pill " + (d.enabled ? "ok" : "bad");

  $("enabled").checked = d.enabled === true;
  $("target").max = max;
  $("target").value = Math.min(max, Number(settings.target ?? 0));
  $("targetValue").textContent = $("target").value;

  const mode = ["random", "specific", "custom"].includes(settings.nameMode) ? settings.nameMode : "custom";
  setNameMode(mode, false);
  $("customNames").value = settings.customNamesText ?? "";
  $("dbCount").textContent = d.randomDatabaseCount ?? databaseNames.length;

  $("replaceEnabled").checked = settings.replaceEnabled === true;
  $("replaceAt").max = max;
  $("replaceAt").value = Math.min(max, Math.max(1, Number(settings.replaceAt ?? max)));
  $("replaceValue").textContent = $("replaceAt").value;
  $("showTab").checked = settings.showTab !== false;

  $("count").textContent = `${bots.length} bot${bots.length === 1 ? "" : "s"}`;
  $("rows").innerHTML = bots.length
    ? bots.map(x => `<div class="item"><span>${esc(x.name)}</span><span>${esc(x.id)}</span><span>${esc(x.ping)} ms</span></div>`).join("")
    : '<div class="empty">Não há jogadores virtuais apresentados.</div>';

  renderSpecificNames();
  updateConditionals();
};

$("nameModeButton").addEventListener("click", e => {
  e.stopPropagation();
  const menu = $("nameModeMenu");
  const willOpen = menu.classList.contains("hidden");
  menu.classList.toggle("hidden", !willOpen);
  $("nameModeButton").setAttribute("aria-expanded", willOpen ? "true" : "false");
});

$("nameModeMenu").querySelectorAll(".modeSelectOption").forEach(option => {
  option.addEventListener("click", () => setNameMode(option.dataset.value));
});

$("nameModeButton").addEventListener("keydown", e => {
  if (e.key === "ArrowDown") { e.preventDefault(); stepNameMode(1); }
  else if (e.key === "ArrowUp") { e.preventDefault(); stepNameMode(-1); }
  else if (e.key === "Enter" || e.key === " ") {
    e.preventDefault();
    $("nameModeButton").click();
  } else if (e.key === "Escape") {
    e.preventDefault();
    closeNameModeMenu();
  }
});

document.addEventListener("click", e => {
  if (!$("nameModeControl").contains(e.target)) closeNameModeMenu();
});

$("target").addEventListener("input", () => { $("targetValue").textContent = $("target").value; updateSpecificCount(); });
$("replaceAt").addEventListener("input", () => { $("replaceValue").textContent = $("replaceAt").value; updateSpecificCount(); });
$("replaceEnabled").addEventListener("change", () => { updateConditionals(); updateSpecificCount(); });
$("specificSearch").addEventListener("input", renderSpecificNames);
$("selectVisible").addEventListener("click", () => {
  const q = ($("specificSearch").value || "").trim().toLocaleLowerCase("pt-PT");
  databaseNames.filter(name => !q || name.toLocaleLowerCase("pt-PT").includes(q)).forEach(name => selectedSpecific.add(name));
  renderSpecificNames();
});
$("clearSpecific").addEventListener("click", () => { selectedSpecific.clear(); renderSpecificNames(); });
$("close").addEventListener("click", () => send("playerbots:uiClose"));
$("refresh").addEventListener("click", () => send("playerbots:uiRefresh"));
$("save").addEventListener("click", () => send("playerbots:uiSave", JSON.stringify({
  enabled: $("enabled").checked,
  target: Number($("target").value),
  nameMode: $("nameMode").value,
  customNamesText: $("customNames").value,
  specificNames: databaseNames.filter(name => selectedSpecific.has(name)),
  replaceEnabled: $("replaceEnabled").checked,
  replaceAt: Number($("replaceAt").value),
  showTab: $("showTab").checked
})));

document.addEventListener("keydown", e => {
  if (e.key === "Escape") send("playerbots:uiClose");
});
