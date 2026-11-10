"use strict";

// COMP 360 — NC A&T frontend. All student/course operations go through the
// local Python API, which forwards them to the team's unchanged C++ backend.
const state = { students: [], courses: [], selectedId: "", registrationId: "", connected: false };
const $ = (id) => document.getElementById(id);
const escapeHtml = (value) => String(value ?? "").replace(/[&<>"']/g, (c) => ({
  "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;"
}[c]));
const label = (s) => `${s.firstName} ${s.lastName} (${s.id})`;
const icon = (name) => `<svg class="icon" aria-hidden="true"><use href="#i-${name}"/></svg>`;

let toastTimer;
function toast(message, isError = false) {
  const el = $("toast");
  el.textContent = message;
  el.className = `toast show${isError ? " error" : ""}`;
  clearTimeout(toastTimer);
  toastTimer = setTimeout(() => { el.className = "toast"; }, 4500);
}

async function api(path, data) {
  const options = data === undefined ? {} : {
    method: "POST", headers: { "Content-Type": "application/json" }, body: JSON.stringify(data)
  };
  let response;
  try { response = await fetch(path, options); }
  catch { throw new Error("Cannot connect to the app server. Start it using start_windows.bat or py server.py."); }
  let result;
  try { result = await response.json(); }
  catch { throw new Error("The server returned an invalid response."); }
  if (!response.ok || !result.ok) throw new Error(result.error || "Unable to complete the request.");
  return result;
}

function setConnection(connected) {
  state.connected = connected;
  $("connection-label").textContent = connected ? "Backend connected" : "Backend unavailable";
  $("connection-dot").style.background = connected ? "#49c98c" : "#ec8a7d";
}

function navigate(page) {
  for (const node of document.querySelectorAll(".page")) node.classList.toggle("active", node.id === `page-${page}`);
  for (const node of document.querySelectorAll(".nav-link")) node.classList.toggle("active", node.dataset.nav === page);
  $("sidebar").classList.remove("open");
  $("sidebar-overlay").classList.remove("visible");
  if (page === "records") renderRecords();
  if (page === "registration") renderRegistrationSummary();
  window.scrollTo({ top: 0, behavior: "auto" });
}

function renderCatalog(tbody) {
  tbody.innerHTML = state.courses.length ? state.courses.map((c) => `
    <tr><td class="course-code">${escapeHtml(c.code)}</td><td>${escapeHtml(c.name)}</td><td>${escapeHtml(c.credits)}</td></tr>
  `).join("") : `<tr><td colspan="3" class="empty-cell">No available courses.</td></tr>`;
}

function populateSelect(select, students, placeholder) {
  const old = select.value;
  select.innerHTML = `<option value="">${escapeHtml(placeholder)}</option>` +
    students.map((s) => `<option value="${escapeHtml(s.id)}">${escapeHtml(label(s))}</option>`).join("");
  if (students.some((s) => s.id === old)) select.value = old;
}

function renderSelectors() {
  populateSelect($("register-student"), state.students, "Select a student...");
  populateSelect($("grade-student"), state.students, "Select a student...");
  populateSelect($("project-student"), state.students.filter((s) => s.type === "MS" || s.type === "PhD"), "Select an MS or PhD student...");
  populateSelect($("dissertation-student"), state.students.filter((s) => s.type === "PhD"), "Select a PhD student...");

  const registerCourse = $("register-course");
  const oldCourse = registerCourse.value;
  registerCourse.innerHTML = `<option value="">Select a course...</option>` +
    state.courses.map((c) => `<option value="${escapeHtml(c.code)}">${escapeHtml(c.code)} — ${escapeHtml(c.name)}</option>`).join("");
  if (state.courses.some((c) => c.code === oldCourse)) registerCourse.value = oldCourse;
  renderGradeCourses();
}

function renderGradeCourses() {
  const select = $("grade-course");
  const selectedStudent = state.students.find((s) => s.id === $("grade-student").value);
  const old = select.value;
  const registered = selectedStudent?.courses || [];
  select.innerHTML = `<option value="">Select a registered course...</option>` +
    registered.map((c) => `<option value="${escapeHtml(c.code)}">${escapeHtml(c.code)} — ${escapeHtml(c.name)}</option>`).join("");
  if (registered.some((c) => c.code === old)) select.value = old;
}

function renderDashboard() {
  $("stat-total").textContent = state.students.length;
  $("stat-ug").textContent = state.students.filter((s) => s.type === "Undergraduate").length;
  $("stat-ms").textContent = state.students.filter((s) => s.type === "MS").length;
  $("stat-phd").textContent = state.students.filter((s) => s.type === "PhD").length;
  $("course-count").textContent = `${state.courses.length} courses`;
  renderCatalog($("catalog-body"));
  renderCatalog($("registration-catalog"));
}

function typeBadge(type) {
  const css = type === "PhD" ? "phd" : type === "MS" ? "ms" : "";
  const display = type === "Undergraduate" ? "BS" : type;
  return `<span class="type-badge ${css}">${escapeHtml(display)}</span>`;
}

function initials(s) { return escapeHtml(`${(s.firstName || "?")[0]}${(s.lastName || "?")[0]}`.toUpperCase()); }

function renderRecords() {
  const query = $("records-search").value.trim().toLowerCase();
  const students = state.students.filter((s) =>
    `${s.firstName} ${s.lastName} ${s.id} ${s.type}`.toLowerCase().includes(query));
  $("records-count").textContent = `${state.students.length} students`;
  $("student-list").innerHTML = students.length ? students.map((s) => `
    <button class="student-item ${state.selectedId === s.id ? "selected" : ""}" data-student-id="${escapeHtml(s.id)}" type="button">
      <span class="student-avatar">${initials(s)}</span>
      <span class="student-item-body"><strong>${escapeHtml(s.firstName)} ${escapeHtml(s.lastName)}</strong>
      <small>ID: ${escapeHtml(s.id)}</small></span>${typeBadge(s.type)}
    </button>
  `).join("") : `<div class="list-empty">${query ? "No matching students." : "No students added yet."}</div>`;
  renderStudentDetail();
}

function gradeLabel(grade) {
  return grade && grade !== "N/A" && grade !== "Not graded" ? grade : "Not graded";
}

function renderStudentDetail() {
  const s = state.students.find((item) => item.id === state.selectedId);
  if (!s) {
    $("record-detail").className = "record-placeholder";
    $("record-detail").innerHTML = `<div class="placeholder-icon">${icon("users")}</div><strong>No student selected</strong><p>Choose a student from the directory to view their information.</p>`;
    return;
  }
  $("record-detail").className = "detail-body";
  const courses = s.courses || [];
  const courseHtml = courses.length ? courses.map((c) => `
    <div class="detail-course"><strong>${escapeHtml(c.code)}</strong><span>${escapeHtml(c.name)} (${escapeHtml(c.credits)} credits)</span><span class="detail-grade">${escapeHtml(gradeLabel(c.grade))}</span></div>`).join("") : `<div class="summary-empty">No registered courses.</div>`;
  $("record-detail").innerHTML = `
    <div class="detail-header"><div class="detail-avatar">${initials(s)}</div><div><h3>${escapeHtml(s.firstName)} ${escapeHtml(s.lastName)}</h3><p>Student ID: ${escapeHtml(s.id)} · ${escapeHtml(s.type)}</p></div></div>
    <div class="detail-section"><p class="detail-section-title">Personal information</p><div class="detail-info">
      <div><small>First Name</small><strong>${escapeHtml(s.firstName)}</strong></div><div><small>Last Name</small><strong>${escapeHtml(s.lastName)}</strong></div>
      <div><small>Phone</small><strong>${escapeHtml(s.phone)}</strong></div><div><small>Student Type</small><strong>${escapeHtml(s.type)}</strong></div>
      <div style="grid-column:1/-1"><small>Address</small><strong>${escapeHtml(s.address)}</strong></div></div></div>
    <div class="detail-section"><p class="detail-section-title">Academic performance</p>
      <div class="gpa-box"><span>Current GPA</span><strong>${Number(s.gpa || 0).toFixed(2)} <span>/ 4.00</span></strong></div>
      <p class="detail-section-title" style="margin-top:22px">Registered courses (${courses.length})</p>${courseHtml}</div>
    ${s.type !== "Undergraduate" ? `<div class="detail-section"><p class="detail-section-title">Research work</p><div class="detail-info">
      <div style="grid-column:1/-1"><small>MS Project</small><strong>${escapeHtml(s.msProject || "Not registered")}</strong></div>
      ${s.type === "PhD" ? `<div style="grid-column:1/-1"><small>PhD Dissertation</small><strong>${escapeHtml(s.dissertation || "Not registered")}</strong></div>` : ""}
    </div></div>` : ""}
  `;
}

function renderRegistrationSummary() {
  const id = state.registrationId || $("register-student").value;
  const s = state.students.find((item) => item.id === id);
  const target = $("registration-summary");
  if (!s) {
    target.className = "result-empty";
    target.innerHTML = `${icon("book")}<p>Select a student to view their registered courses.</p>`;
    return;
  }
  target.className = "summary-body";
  const rows = (s.courses || []).map((c) => `
    <div class="summary-course"><div><strong>${escapeHtml(c.code)} — ${escapeHtml(c.name)}</strong><small>${escapeHtml(c.credits)} credit hours</small></div><span class="tag-grade">${escapeHtml(gradeLabel(c.grade))}</span></div>
  `).join("") || `<p class="summary-empty">No registered courses yet.</p>`;
  target.innerHTML = `
    <div class="summary-meta"><div><div class="meta-name">${escapeHtml(s.firstName)} ${escapeHtml(s.lastName)}</div><div class="meta-id">Student ID: ${escapeHtml(s.id)}</div></div>${typeBadge(s.type)}</div>
    <div class="summary-title">CURRENTLY REGISTERED COURSES</div>${rows}
    <div class="summary-footer"><span>${(s.courses || []).length} courses registered</span><button type="button" class="print-button" id="print-summary">${icon("printer")} Print summary</button></div>
  `;
  $("print-summary").addEventListener("click", () => window.print());
}

async function loadData(showToast = false) {
  try {
    const [courses, students] = await Promise.all([api("/api/courses"), api("/api/students")]);
    state.courses = courses.courses || [];
    state.students = students.students || [];
    setConnection(true);
    renderDashboard();
    renderSelectors();
    renderRecords();
    renderRegistrationSummary();
    if (showToast) toast("Student and course data refreshed.");
  } catch (error) {
    setConnection(false);
    toast(error.message, true);
  }
}

function payload(form) { return Object.fromEntries(new FormData(form).entries()); }
function wireForm(id, endpoint, onSuccess) {
  const form = $(id);
  form.addEventListener("submit", async (event) => {
    event.preventDefault();
    if (!form.reportValidity()) return;
    const submit = form.querySelector('[type="submit"]');
    const data = payload(form);
    submit.disabled = true;
    try {
      const response = await api(endpoint, data);
      if (onSuccess) await onSuccess(data, response);
      await loadData();
      toast(response.message || "Saved successfully.");
    } catch (error) {
      toast(error.message, true);
    } finally {
      submit.disabled = false;
    }
  });
}

function initialize() {
  document.querySelectorAll("[data-nav]").forEach((button) =>
    button.addEventListener("click", () => navigate(button.dataset.nav)));
  $("mobile-menu").addEventListener("click", () => {
    $("sidebar").classList.add("open"); $("sidebar-overlay").classList.add("visible");
  });
  $("sidebar-overlay").addEventListener("click", () => {
    $("sidebar").classList.remove("open"); $("sidebar-overlay").classList.remove("visible");
  });
  $("refresh-dashboard").addEventListener("click", () => loadData(true));
  $("records-search").addEventListener("input", renderRecords);
  $("student-list").addEventListener("click", (event) => {
    const button = event.target.closest("[data-student-id]");
    if (!button) return;
    state.selectedId = button.dataset.studentId;
    renderRecords();
  });
  $("register-student").addEventListener("change", (event) => {
    state.registrationId = event.target.value;
    renderRegistrationSummary();
  });
  $("grade-student").addEventListener("change", renderGradeCourses);
  wireForm("add-student-form", "/api/students", async (data) => {
    state.selectedId = data.id;
    $("add-student-form").reset();
    navigate("records");
  });
  wireForm("registration-form", "/api/register-course", async (data, response) => {
    state.registrationId = data.id;
    // The backend response includes the full updated registration summary.
    if (response.student) {
      const i = state.students.findIndex((s) => s.id === data.id);
      if (i >= 0) state.students[i] = response.student;
    }
  });
  wireForm("grade-form", "/api/grade", async (data) => { state.selectedId = data.id; });
  wireForm("project-form", "/api/project", async (data) => {
    state.selectedId = data.id; $("project-title").value = "";
  });
  wireForm("dissertation-form", "/api/dissertation", async (data) => {
    state.selectedId = data.id; $("dissertation-title").value = "";
  });
  loadData();
}

document.addEventListener("DOMContentLoaded", initialize);
