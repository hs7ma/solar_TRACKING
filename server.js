const express = require('express');
const path = require('path');

const app = express();
const PORT = process.env.PORT || 3000;

app.use(express.json());
app.use(express.static(path.join(__dirname)));

let latestReading = null;

app.post('/api/data', (req, res) => {
  const { voltage, current, power, panAngle, tiltAngle, ldrTL, ldrTR, ldrBL, ldrBR } = req.body || {};

  latestReading = {
    voltage: parseFloat(voltage) || 0,
    current: parseFloat(current) || 0,
    power: parseFloat(power) || 0,
    panAngle: parseInt(panAngle) || 0,
    tiltAngle: parseInt(tiltAngle) || 0,
    ldrTL: parseInt(ldrTL) || 0,
    ldrTR: parseInt(ldrTR) || 0,
    ldrBL: parseInt(ldrBL) || 0,
    ldrBR: parseInt(ldrBR) || 0,
    timestamp: Date.now()
  };

  res.json({ success: true, data: latestReading });
});

app.get('/api/data', (req, res) => {
  res.setHeader('Access-Control-Allow-Origin', '*');

  if (!latestReading) {
    res.json({
      voltage: 0,
      current: 0,
      power: 0,
      panAngle: 0,
      tiltAngle: 0,
      ldrTL: 0,
      ldrTR: 0,
      ldrBL: 0,
      ldrBR: 0,
      timestamp: null,
      status: 'waiting'
    });
    return;
  }

  res.json(latestReading);
});

app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, 'index.html'));
});

app.listen(PORT, () => {
  console.log(`Solar Tracker Server running on port ${PORT}`);
});