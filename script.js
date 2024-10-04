// Get references to DOM elements
const espList = document.getElementById('esp-list');
const addBtn = document.getElementById('add-btn');
const espNameInput = document.getElementById('esp-name');
const espIpInput = document.getElementById('esp-ip');
const addError = document.getElementById('add-error');
const modal = document.getElementById('modal');
const closeModalBtn = document.querySelector('.close');
const selectedEsp = document.getElementById('selected-esp');
const openDoorBtn = document.getElementById('open-door');
const closeDoorBtn = document.getElementById('close-door');

// Removal modal elements
const removeModal = document.getElementById('remove-modal');
const adminPasswordInput = document.getElementById('admin-password');
const passwordError = document.getElementById('password-error');
const confirmRemoveBtn = document.getElementById('confirm-remove-btn');
const removeCloseBtn = document.querySelector('.remove-close');

let espModules = [];
let moduleToRemove = null;  // Track the module pending removal

// Load ESP32 modules from local storage on page load
const storedEspModules = localStorage.getItem('espModules');
if (storedEspModules) {
  try {
    espModules = JSON.parse(storedEspModules);
  } catch (error) {
    console.error('Error parsing stored ESP32 modules:', error);
  }
}

// Update ESP32 module list on page load
updateEspList();

// Add ESP32 module
addBtn.addEventListener('click', () => {
  const espName = espNameInput.value;
  const espIp = espIpInput.value;

  if (!espName || !espIp) {
    addError.textContent = 'Please enter ESP32 name and IP address.';
    return;
  }

  const module = { name: espName, ip: espIp };
  espModules.push(module);

  // Store updated list in local storage
  localStorage.setItem('espModules', JSON.stringify(espModules));

  addError.textContent = '';
  updateEspList();
  espNameInput.value = '';
  espIpInput.value = '';
});

// Add ESP32 module to list
function addEspModule(module) {
  const li = document.createElement('li');
  li.innerHTML = `${module.name} (${module.ip}) 
                  <button class="control-btn btn primary-btn">Control</button>
                  <button class="remove-btn btn danger-btn">Remove</button>`;

  // Control button click event
  li.querySelector('.control-btn').addEventListener('click', () => showControlPanel(module));
  
  // Remove button click event
  li.querySelector('.remove-btn').addEventListener('click', () => removeEspModule(module));

  espList.appendChild(li);
}

// Show control panel for selected ESP32
function showControlPanel(module) {
  selectedEsp.textContent = module.name;
  modal.classList.add('open');
  openDoorBtn.onclick = () => controlRelay(module.ip, 'open');
  closeDoorBtn.onclick = () => controlRelay(module.ip, 'close');
}

// Remove ESP32 module
function removeEspModule(module) {
  // Show confirmation modal
  confirmRemoveEspModule(module);
}

// Show password confirmation modal before removing the ESP32 module
function confirmRemoveEspModule(module) {
  moduleToRemove = module;  // Store the module to be removed
  removeModal.classList.add('open');
}

// Remove ESP32 module if the password is correct
confirmRemoveBtn.addEventListener('click', () => {
  const enteredPassword = adminPasswordInput.value;
  
  if (enteredPassword === '1234') {
    // Password is correct, proceed to remove the module
    espModules = espModules.filter(m => m.ip !== moduleToRemove.ip);
    localStorage.setItem('espModules', JSON.stringify(espModules));
    updateEspList();
    
    // Clear input and close modal
    adminPasswordInput.value = '';
    removeModal.classList.remove('open');
    passwordError.textContent = '';
  } else {
    // Incorrect password, show error
    passwordError.textContent = 'Incorrect password. Please try again.';
  }
});

// Close modal
closeModalBtn.addEventListener('click', () => {
  modal.classList.remove('open');
});

removeCloseBtn.addEventListener('click', () => {
  removeModal.classList.remove('open');
  passwordError.textContent = '';  // Clear previous error messages
  adminPasswordInput.value = '';   // Reset password field
});

// Update ESP32 module list in UI
function updateEspList() {
  espList.innerHTML = '';
  espModules.forEach(module => addEspModule(module));
}

// Control relay by sending HTTP request
function controlRelay(ip, action) {
  const url = `http://${ip}/relay/${action}`;
  fetch(url)
    .then(response => {
      if (response.ok) {
        console.log(`Successfully sent ${action} command to ${ip}`);
      } else {
        console.error('Failed to control relay:', response.status);
      }
    })
    .catch(error => console.error('Error:', error));
}
