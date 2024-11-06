effect_commands <- read.csv("data-raw/effects.csv", fileEncoding = "UTF-8")
save(effect_commands, file = "data/effect_commands.rdata", compress = TRUE)
