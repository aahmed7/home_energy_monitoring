if (flow.get("volt_max") === undefined) {
  flow.set("volt_max", msg.payload);
}
if (flow.get("volt_min") === undefined) {
  flow.set("volt_min", msg.payload);
}
if (flow.get("volt_mean") === undefined) {
  flow.set("volt_mean", msg.payload);
}

volt_max = flow.get("volt_max");
volt_min = flow.get("volt_min");
volt_mean = flow.get("volt_mean");

temp = parseInt(msg.payload);

if (volt_max < temp) {
  flow.set("volt_max", temp);
  msg.max = volt_max;
} else {
  msg.max = volt_max;
}

if (volt_min > temp) {
    flow.set("volt_min", temp);
    msg.min = volt_min;
  } else {
    msg.min = volt_min;
  }

return msg;
