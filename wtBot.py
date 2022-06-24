import discord, asyncio, os
from discord.ext import commands

game = discord.Game("Weather Broadcasting")
bot = commands.Bot(command_prefix = '!', status = discord.Status.online, activity = game)

@bot.command(aliases = ['안녕'])
async def hello(ctx):
    await ctx.send(f'hello!')

@bot.command(aliases = ['날씨'])
async def weather(ctx):
    f1 = open("C:/cansat_code/cansatData.txt", 'rt', encoding = 'UTF-8')
    count = 0
    while 1:
        if f1.readline() == '':
            break
        count += 1
    f1.close()
    f2 = open("C:/cansat_code/cansatData.txt", 'rt', encoding = 'UTF-8')
    data = f2.readlines()[count - 4 : count]
    time = data[0]
    pos = data[1]
    atmo = data[2]
    rain = data[3]
    embed = discord.Embed(title = "기상 정보", description = '', color = 0x4432a8)
    embed.add_field(name = '시각', value = time, inline = False)
    embed.add_field(name = '위치', value = pos, inline = False)
    embed.add_field(name = '대기 상황', value = atmo, inline = False)
    embed.add_field(name = '강수', value = rain, inline = False)
    f2.close()
    await ctx.send(embed = embed)

@bot.command(aliases = ['동영상'])
async def cam(ctx):
    embed = discord.Embed(title = "기상 웹 스트리밍", url = "https://192.0.0.1", color = 0x4432a8)
    await ctx.send(embed = embed)

bot.run('ENTER_THE_TOKEN_OF_YOUR_BOT')