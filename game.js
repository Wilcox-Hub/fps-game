(() => {
  const canvas = document.querySelector('#game-canvas');
  const ctx = canvas.getContext('2d');
  const $ = (selector) => document.querySelector(selector);
  const ui = {
    hud: $('#hud'), start: $('#start-screen'), pause: $('#pause-screen'), over: $('#gameover-screen'),
    health: $('#health-text'), healthFill: $('#health-fill'), healthCard: $('.health-card'),
    ammo: $('#ammo-text'), reserve: $('#reserve-text'), ammoFill: $('#ammo-fill'), score: $('#score-text'),
    wave: $('#wave-text'), objective: $('#objective'), toast: $('#toast'), hit: $('#hit-marker'),
    crosshair: $('#crosshair'), finalScore: $('#final-score'), finalWave: $('#final-wave'), best: $('#best-score')
  };
  const world = [
    '################', '#......#.......#', '#......#.......#', '#..............#',
    '#..##..........#', '#..##....##....#', '#..............#', '#.....##.......#',
    '#.....##.......#', '#..............#', '#....##.....#..#', '#....##.....#..#',
    '#..............#', '#.......#......#', '#..............#', '################'
  ];
  const spawnPoints = [
    { x: 13.5, y: 2.5 }, { x: 14.5, y: 7.5 }, { x: 12.5, y: 13.5 },
    { x: 3.5, y: 13.5 }, { x: 8.5, y: 5.5 }, { x: 10.5, y: 9.5 },
    { x: 2.5, y: 8.5 }, { x: 13.5, y: 11.5 }, { x: 6.5, y: 3.5 }
  ];
  const keys = new Set();
  const touchMoves = new Set();
  const player = { x: 2.5, y: 2.5, angle: 0, health: 100, ammo: 12, reserve: 72, score: 0, wave: 1, kits: 2 };
  const fov = Math.PI / 2.9;
  let enemies = [];
  let gameState = 'menu';
  let width = 0;
  let height = 0;
  let pixelRatio = 1;
  let lastTime = 0;
  let shotCooldown = 0;
  let reloadTime = 0;
  let fireHeld = false;
  let muzzleFlash = 0;
  let toastTimer = 0;
  let touchAimX = null;
  let soundEnabled = true;
  let audioContext;
  let bestScore = 0;
  try { bestScore = Number(localStorage.getItem('neon-breach-best') || 0); } catch {}
  ui.best.textContent = 'BEST ' + String(bestScore).padStart(6, '0');

  function resize() {
    const bounds = canvas.getBoundingClientRect();
    pixelRatio = Math.min(window.devicePixelRatio || 1, 2);
    width = bounds.width;
    height = bounds.height;
    canvas.width = Math.max(1, Math.floor(width * pixelRatio));
    canvas.height = Math.max(1, Math.floor(height * pixelRatio));
    ctx.setTransform(pixelRatio, 0, 0, pixelRatio, 0, 0);
    render();
  }

  function isWall(x, y) {
    const col = Math.floor(x);
    const row = Math.floor(y);
    return row < 0 || row >= world.length || col < 0 || col >= world[0].length || world[row][col] === '#';
  }

  function movePlayer(dx, dy) {
    const radius = 0.19;
    if (!isWall(player.x + dx + Math.sign(dx) * radius, player.y)) player.x += dx;
    if (!isWall(player.x, player.y + dy + Math.sign(dy) * radius)) player.y += dy;
  }

  function normalizeAngle(angle) {
    while (angle > Math.PI) angle -= Math.PI * 2;
    while (angle < -Math.PI) angle += Math.PI * 2;
    return angle;
  }

  function castRay(angle) {
    const rayX = Math.cos(angle);
    const rayY = Math.sin(angle);
    let mapX = Math.floor(player.x);
    let mapY = Math.floor(player.y);
    const deltaX = Math.abs(1 / (rayX || 0.000001));
    const deltaY = Math.abs(1 / (rayY || 0.000001));
    const stepX = rayX < 0 ? -1 : 1;
    const stepY = rayY < 0 ? -1 : 1;
    let sideX = (rayX < 0 ? player.x - mapX : mapX + 1 - player.x) * deltaX;
    let sideY = (rayY < 0 ? player.y - mapY : mapY + 1 - player.y) * deltaY;
    let side = 0;
    let distance = 18;
    for (let count = 0; count < 64; count += 1) {
      if (sideX < sideY) { sideX += deltaX; mapX += stepX; side = 0; }
      else { sideY += deltaY; mapY += stepY; side = 1; }
      if (mapY < 0 || mapY >= world.length || mapX < 0 || mapX >= world[0].length || world[mapY][mapX] === '#') {
        distance = side === 0 ? sideX - deltaX : sideY - deltaY;
        break;
      }
    }
    const hitX = player.x + rayX * distance;
    const hitY = player.y + rayY * distance;
    return { distance: Math.max(0.12, distance * Math.cos(angle - player.angle)), side, texture: side === 0 ? hitY % 1 : hitX % 1 };
  }

  function drawWorld() {
    const sky = ctx.createLinearGradient(0, 0, 0, height * 0.54);
    sky.addColorStop(0, '#101c2c'); sky.addColorStop(1, '#352b43');
    ctx.fillStyle = sky; ctx.fillRect(0, 0, width, height * 0.53);
    const floor = ctx.createLinearGradient(0, height * 0.49, 0, height);
    floor.addColorStop(0, '#273038'); floor.addColorStop(1, '#090e14');
    ctx.fillStyle = floor; ctx.fillRect(0, height * 0.49, width, height * 0.51);
    const strip = 2;
    const depth = new Float32Array(Math.ceil(width / strip));
    for (let ray = 0; ray < depth.length; ray += 1) {
      const screenX = ray * strip;
      const hit = castRay(player.angle + (screenX / width - 0.5) * fov);
      depth[ray] = hit.distance;
      const wallHeight = Math.min(height * 1.6, height * 0.88 / hit.distance);
      const y = (height - wallHeight) / 2;
      const light = Math.max(0.12, 1 - hit.distance / 12) * (hit.side ? 0.66 : 1);
      const seam = Math.floor(Math.abs(hit.texture) * 8) % 8 === 0 ? 0.8 : 1;
      const glow = Math.round(32 + light * 42 * seam);
      ctx.fillStyle = 'rgb(' + Math.round(17 + glow * 0.18) + ',' + Math.round(27 + glow * 0.62) + ',' + Math.round(39 + glow * 0.8) + ')';
      ctx.fillRect(screenX, y, strip + 1, wallHeight);
      if (Math.abs(hit.texture) < 0.025 || Math.abs(hit.texture) > 0.975) {
        ctx.fillStyle = 'rgba(112,240,223,' + (light * 0.62) + ')';
        ctx.fillRect(screenX, y, strip + 1, Math.max(1, wallHeight * 0.018));
      }
      if (seam < 1) { ctx.fillStyle = 'rgba(3,7,11,' + (0.14 + (1 - light) * 0.2) + ')'; ctx.fillRect(screenX, y, strip + 1, wallHeight); }
    }
    drawEnemies(depth, strip);
    const vignette = ctx.createRadialGradient(width / 2, height / 2, height * 0.12, width / 2, height / 2, width * 0.68);
    vignette.addColorStop(0, 'rgba(3,6,12,0)'); vignette.addColorStop(1, 'rgba(3,6,12,.56)');
    ctx.fillStyle = vignette; ctx.fillRect(0, 0, width, height);
    if (muzzleFlash > 0) { ctx.fillStyle = 'rgba(255,220,148,' + (muzzleFlash * 0.12) + ')'; ctx.fillRect(0, 0, width, height); }
  }

  function drawEnemies(depth, strip) {
    const plane = width / (2 * Math.tan(fov / 2));
    const visible = enemies.map((enemy) => {
      const dx = enemy.x - player.x;
      const dy = enemy.y - player.y;
      return { enemy, distance: Math.hypot(dx, dy), angle: normalizeAngle(Math.atan2(dy, dx) - player.angle) };
    }).filter((item) => Math.abs(item.angle) < fov * 0.72 && item.distance > 0.2).sort((a, b) => b.distance - a.distance);
    for (const item of visible) {
      const screenX = width / 2 + Math.tan(item.angle) * plane;
      const scale = Math.min(height * 0.72, height * 0.7 / item.distance);
      const spriteWidth = scale * (item.enemy.kind === 'brute' ? 0.74 : 0.52);
      const ray = Math.max(0, Math.min(depth.length - 1, Math.floor(screenX / strip)));
      if (depth[ray] < item.distance - 0.18) continue;
      const ground = height * 0.52 + scale * 0.38;
      const top = ground - scale * 0.78;
      const enemy = item.enemy;
      ctx.save();
      ctx.shadowColor = enemy.hit > 0 ? '#fff0c7' : '#ff527f';
      ctx.shadowBlur = Math.max(4, scale * 0.14);
      ctx.fillStyle = enemy.hit > 0 ? '#fff0c7' : (enemy.kind === 'brute' ? '#d7487f' : '#ff527f');
      ctx.beginPath();
      ctx.moveTo(screenX - spriteWidth * 0.36, ground - scale * 0.12);
      ctx.lineTo(screenX - spriteWidth * 0.27, top + scale * 0.28);
      ctx.lineTo(screenX - spriteWidth * 0.17, top + scale * 0.1);
      ctx.lineTo(screenX + spriteWidth * 0.17, top + scale * 0.1);
      ctx.lineTo(screenX + spriteWidth * 0.27, top + scale * 0.28);
      ctx.lineTo(screenX + spriteWidth * 0.36, ground - scale * 0.12);
      ctx.closePath(); ctx.fill();
      ctx.fillStyle = '#212b37'; ctx.fillRect(screenX - spriteWidth * 0.22, top + scale * 0.34, spriteWidth * 0.44, scale * 0.3);
      ctx.fillStyle = '#141a22'; ctx.beginPath(); ctx.ellipse(screenX, top + scale * 0.21, spriteWidth * 0.29, scale * 0.2, 0, 0, Math.PI * 2); ctx.fill();
      ctx.fillStyle = '#f8c673'; ctx.fillRect(screenX - spriteWidth * 0.15, top + scale * 0.2, spriteWidth * 0.3, Math.max(2, scale * 0.035));
      ctx.restore();
      const barWidth = spriteWidth * 0.72;
      ctx.fillStyle = '#04070bb3'; ctx.fillRect(screenX - barWidth / 2, top - 8, barWidth, 3);
      ctx.fillStyle = '#ff527f'; ctx.fillRect(screenX - barWidth / 2, top - 8, barWidth * Math.max(0, enemy.hp / enemy.maxHp), 3);
    }
  }

  function showToast(message) {
    ui.toast.textContent = message;
    ui.toast.classList.add('show');
    toastTimer = 1.4;
  }

  function playTone(frequency, duration, type, volume) {
    if (!soundEnabled) return;
    try {
      audioContext ||= new AudioContext();
      if (audioContext.state === 'suspended') audioContext.resume();
      const oscillator = audioContext.createOscillator();
      const gain = audioContext.createGain();
      oscillator.type = type || 'sine';
      oscillator.frequency.setValueAtTime(frequency, audioContext.currentTime);
      gain.gain.setValueAtTime(volume || 0.035, audioContext.currentTime);
      gain.gain.exponentialRampToValueAtTime(0.001, audioContext.currentTime + duration);
      oscillator.connect(gain).connect(audioContext.destination);
      oscillator.start(); oscillator.stop(audioContext.currentTime + duration);
    } catch {}
  }

  function updateHud() {
    ui.health.textContent = String(Math.ceil(player.health));
    ui.healthFill.style.width = Math.max(0, player.health) + '%';
    ui.healthCard.classList.toggle('low', player.health <= 30);
    ui.ammo.textContent = String(player.ammo).padStart(2, '0');
    ui.reserve.textContent = String(player.reserve).padStart(2, '0');
    ui.ammoFill.style.width = (player.ammo / 12) * 100 + '%';
    ui.score.textContent = String(player.score).padStart(6, '0');
  }

  function setState(state) {
    gameState = state;
    ui.start.hidden = state !== 'menu';
    ui.pause.hidden = state !== 'paused';
    ui.over.hidden = state !== 'gameover';
    ui.hud.hidden = state !== 'playing';
    if (state !== 'playing') {
      fireHeld = false;
      if (document.pointerLockElement === canvas) document.exitPointerLock();
    }
  }

  function spawnWave() {
    const count = Math.min(3 + player.wave * 2, 14);
    const points = [...spawnPoints].sort(() => Math.random() - 0.5);
    enemies = [];
    for (let index = 0; index < count; index += 1) {
      const point = points[index % points.length];
      const kind = player.wave >= 3 && index % 4 === 3 ? 'brute' : 'drone';
      const hp = kind === 'brute' ? 4 + Math.floor(player.wave / 2) : 2 + Math.floor(player.wave / 3);
      enemies.push({ x: point.x, y: point.y, hp, maxHp: hp, kind, speed: kind === 'brute' ? 0.42 : 0.66, cooldown: 0.5 + Math.random(), hit: 0 });
    }
    ui.wave.textContent = String(player.wave).padStart(2, '0');
    ui.objective.textContent = 'WAVE ' + String(player.wave).padStart(2, '0') + ' // ' + count + ' HOSTILES';
    showToast(player.wave === 1 ? 'THEY HEARD YOU' : 'WAVE ' + player.wave + ' // HOLD THE LINE');
  }

  function requestPointerLock() {
    if (canvas.requestPointerLock && !matchMedia('(pointer: coarse)').matches) {
      try { const pending = canvas.requestPointerLock(); if (pending && pending.catch) pending.catch(() => {}); } catch {}
    }
  }

  function resetRun() {
    player.x = 2.5; player.y = 2.5; player.angle = 0;
    player.health = 100; player.ammo = 12; player.reserve = 72;
    player.score = 0; player.wave = 1; player.kits = 2;
    reloadTime = 0; shotCooldown = 0;
    updateHud(); spawnWave(); setState('playing'); requestPointerLock();
  }

  function finishRun() {
    setState('gameover');
    ui.finalScore.textContent = String(player.score).padStart(6, '0');
    ui.finalWave.textContent = String(Math.max(0, player.wave - 1)).padStart(2, '0');
    if (player.score > bestScore) {
      bestScore = player.score;
      try { localStorage.setItem('neon-breach-best', String(bestScore)); } catch {}
      ui.best.textContent = 'BEST ' + String(bestScore).padStart(6, '0');
    }
    playTone(110, 0.5, 'triangle', 0.06);
  }

  function clearWave() {
    player.wave += 1; player.reserve += 18; player.health = Math.min(100, player.health + 8);
    if (player.wave % 3 === 0) player.kits = Math.min(3, player.kits + 1);
    updateHud(); showToast('SECTOR CLEAR // +18 CHARGE');
    window.setTimeout(() => { if (gameState === 'playing') spawnWave(); }, 1050);
  }

  function shoot() {
    if (gameState !== 'playing' || shotCooldown > 0 || reloadTime > 0) return;
    if (player.ammo <= 0) { showToast(player.reserve > 0 ? 'RELOAD // R' : 'EMPTY // FIND A CACHE'); shotCooldown = 0.2; return; }
    player.ammo -= 1; shotCooldown = 0.22; muzzleFlash = 1;
    ui.crosshair.classList.add('firing'); window.setTimeout(() => ui.crosshair.classList.remove('firing'), 75);
    playTone(88 + Math.random() * 12, 0.11, 'sawtooth', 0.055);
    const wallDistance = castRay(player.angle).distance;
    let target = null; let targetDistance = Infinity;
    for (const enemy of enemies) {
      const dx = enemy.x - player.x; const dy = enemy.y - player.y;
      const distance = Math.hypot(dx, dy);
      const error = Math.abs(normalizeAngle(Math.atan2(dy, dx) - player.angle));
      const tolerance = Math.max(0.045, Math.atan2(enemy.kind === 'brute' ? 0.38 : 0.27, distance));
      if (error < tolerance && distance < wallDistance + 0.12 && distance < targetDistance) { target = enemy; targetDistance = distance; }
    }
    if (target) {
      target.hp -= 1; target.hit = 0.16;
      ui.hit.classList.add('visible'); window.setTimeout(() => ui.hit.classList.remove('visible'), 180);
      playTone(480, 0.07, 'triangle', 0.045);
      if (target.hp <= 0) {
        enemies = enemies.filter((enemy) => enemy !== target);
        player.score += target.kind === 'brute' ? 220 : 100;
        if (!enemies.length) clearWave();
      }
    }
    updateHud();
  }

  function reload() {
    if (gameState !== 'playing' || reloadTime > 0 || player.ammo === 12 || player.reserve <= 0) return;
    reloadTime = 1.05; showToast('RECHARGING');
  }

  function medkit() {
    if (gameState !== 'playing' || player.kits <= 0 || player.health >= 100) return;
    player.kits -= 1; player.health = Math.min(100, player.health + 38);
    showToast('PATCHED UP // ' + player.kits + ' MED LEFT'); playTone(390, 0.18, 'sine', 0.04); updateHud();
  }

  function updateEnemies(dt) {
    for (const enemy of enemies) {
      const dx = player.x - enemy.x; const dy = player.y - enemy.y; const distance = Math.hypot(dx, dy);
      enemy.hit = Math.max(0, enemy.hit - dt); enemy.cooldown -= dt;
      if (distance > 1.05) {
        const speed = enemy.speed * dt; const mx = dx / distance * speed; const my = dy / distance * speed;
        if (!isWall(enemy.x + mx + Math.sign(mx) * 0.16, enemy.y)) enemy.x += mx;
        if (!isWall(enemy.x, enemy.y + my + Math.sign(my) * 0.16)) enemy.y += my;
      } else if (enemy.cooldown <= 0) {
        enemy.cooldown = 1.15 + Math.random() * 0.55;
        player.health = Math.max(0, player.health - (enemy.kind === 'brute' ? 13 : 8));
        showToast('IMPACT // KEEP MOVING'); playTone(120, 0.1, 'sawtooth', 0.04); updateHud();
        if (player.health <= 0) finishRun();
        break;
      }
    }
  }

  function update(dt) {
    if (gameState !== 'playing') return;
    shotCooldown = Math.max(0, shotCooldown - dt); muzzleFlash = Math.max(0, muzzleFlash - dt * 6);
    if (toastTimer > 0 && (toastTimer -= dt) <= 0) ui.toast.classList.remove('show');
    if (reloadTime > 0 && (reloadTime -= dt) <= 0) {
      const amount = Math.min(12 - player.ammo, player.reserve);
      player.ammo += amount; player.reserve -= amount; updateHud(); playTone(260, 0.07, 'triangle', 0.025);
    }
    let forward = Number(keys.has('KeyW') || keys.has('ArrowUp') || touchMoves.has('forward')) - Number(keys.has('KeyS') || keys.has('ArrowDown') || touchMoves.has('back'));
    let strafe = Number(keys.has('KeyD') || keys.has('ArrowRight') || touchMoves.has('right')) - Number(keys.has('KeyA') || keys.has('ArrowLeft') || touchMoves.has('left'));
    const length = Math.hypot(forward, strafe) || 1; forward /= length; strafe /= length;
    const speed = (keys.has('ShiftLeft') ? 3.25 : 2.45) * dt;
    movePlayer((Math.cos(player.angle) * forward - Math.sin(player.angle) * strafe) * speed,
      (Math.sin(player.angle) * forward + Math.cos(player.angle) * strafe) * speed);
    if (fireHeld) shoot();
    updateEnemies(dt);
  }

  function renderMenu() {
    const gradient = ctx.createLinearGradient(0, 0, width, height);
    gradient.addColorStop(0, '#152638'); gradient.addColorStop(0.53, '#282235'); gradient.addColorStop(1, '#0b1017');
    ctx.fillStyle = gradient; ctx.fillRect(0, 0, width, height);
    ctx.strokeStyle = 'rgba(112,240,223,.14)';
    for (let line = 0; line < 14; line += 1) {
      const y = height * 0.56 + line * line * 0.5;
      ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(width, y); ctx.stroke();
    }
    for (let line = -8; line <= 8; line += 1) {
      ctx.beginPath(); ctx.moveTo(width / 2 + line * 14, height); ctx.lineTo(width / 2 + line * width * 0.12, height * 0.54); ctx.stroke();
    }
  }

  function render() {
    if (!ctx || !width || !height) return;
    if (gameState === 'playing' || gameState === 'paused') drawWorld();
    else renderMenu();
  }

  function frame(time) {
    const dt = Math.min(0.04, (time - lastTime) / 1000 || 0);
    lastTime = time; update(dt); render(); requestAnimationFrame(frame);
  }

  $('#start-button').addEventListener('click', resetRun);
  $('#resume-button').addEventListener('click', () => { setState('playing'); requestPointerLock(); });
  $('#restart-button').addEventListener('click', resetRun);
  $('#play-again-button').addEventListener('click', resetRun);
  $('#sound-button').addEventListener('click', (event) => {
    soundEnabled = !soundEnabled;
    event.currentTarget.setAttribute('aria-pressed', String(soundEnabled));
    event.currentTarget.setAttribute('aria-label', soundEnabled ? 'Mute sound' : 'Enable sound');
  });
  $('#touch-medkit').addEventListener('click', medkit);
  $('#touch-fire').addEventListener('pointerdown', (event) => { event.preventDefault(); fireHeld = true; });
  for (const name of ['pointerup', 'pointercancel', 'pointerleave']) $('#touch-fire').addEventListener(name, () => { fireHeld = false; });
  document.querySelectorAll('[data-move]').forEach((button) => {
    const direction = button.dataset.move;
    button.addEventListener('pointerdown', (event) => { event.preventDefault(); touchMoves.add(direction); });
    for (const name of ['pointerup', 'pointercancel', 'pointerleave']) button.addEventListener(name, () => touchMoves.delete(direction));
  });
  canvas.addEventListener('pointerdown', (event) => {
    if (gameState !== 'playing') return;
    if (event.pointerType === 'touch') { touchAimX = event.clientX; return; }
    fireHeld = true; requestPointerLock();
  });
  canvas.addEventListener('pointermove', (event) => {
    if (touchAimX !== null && event.pointerType === 'touch') {
      player.angle += (event.clientX - touchAimX) * 0.006;
      touchAimX = event.clientX;
    }
  });
  canvas.addEventListener('pointerup', () => { touchAimX = null; });
  canvas.addEventListener('pointercancel', () => { touchAimX = null; });
  window.addEventListener('pointerup', () => { fireHeld = false; });
  document.addEventListener('mousemove', (event) => {
    if (gameState === 'playing' && document.pointerLockElement === canvas) player.angle += event.movementX * 0.0027;
  });
  document.addEventListener('pointerlockchange', () => {
    if (document.pointerLockElement !== canvas && gameState === 'playing' && !matchMedia('(pointer: coarse)').matches) setState('paused');
  });
  window.addEventListener('keydown', (event) => {
    if (['Space', 'ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight'].includes(event.code)) event.preventDefault();
    keys.add(event.code);
    if (gameState === 'menu' && (event.code === 'Enter' || event.code === 'Space')) resetRun();
    if (event.code === 'Escape' && gameState === 'playing') setState('paused');
    if (event.code === 'Escape' && gameState === 'paused') { setState('playing'); requestPointerLock(); }
    if (event.code === 'KeyR') reload();
    if (event.code === 'KeyF' || event.code === 'KeyQ') medkit();
    if (event.code === 'Space' && gameState === 'playing') fireHeld = true;
  });
  window.addEventListener('keyup', (event) => { keys.delete(event.code); if (event.code === 'Space') fireHeld = false; });
  window.addEventListener('blur', () => { keys.clear(); touchMoves.clear(); fireHeld = false; if (gameState === 'playing') setState('paused'); });
  document.addEventListener('visibilitychange', () => { if (document.hidden && gameState === 'playing') setState('paused'); });
  window.addEventListener('resize', resize);
  resize(); setState('menu'); requestAnimationFrame(frame);
})();
