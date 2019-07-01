import os
import time
import json
import CNN_Reversi
import low_level_CNNReversi
#-1:white; 1:black
#0 :white; 1:black
path = '/Users/mro/Desktop/' 

def judge(input_requests,input_responses):
	input = '{"requests":[' + input_requests + '],' + '"responses":[' + input_responses + ']}'
	input = ''.join(input.split())
	#print (input)
	input = input.replace('"','\\"')
	command = '~/Desktop/judge/Reversi/judge'
	output = os.popen(command + ' ' + '"' + input + '"').read()
	result = output.split('\n')
	print(output)
	return (result[-1:][0])

def execute(input_requests,input_responses):
	command = '~/Desktop/Reversi/Reversi/self-play'
	input = '{"requests":[' + input_requests + '],' + '"responses":[' + input_responses + ']}'
	#print(input)
	input = ''.join(input.split())
	input = input.replace('"','\\"')
	output = os.popen(command + ' ' + '"' + input + '"').read()
	#print(output)
	try:
		result = json.loads(output)
		result = result['response']
		result = json.dumps(result)
		return result
	except Exception as e:
		return None

def play_once():
	turnId = 1
	first_to_play = '{"x":-1,"y":-1}'
	requests = []
	responses = []

	input_requests = '{"x":-1,"y":-1}'
	input_responses = ''
	response = execute(input_requests,input_responses)
	responses.append(response)
	turnId += 1	
	while (judge(input_requests,input_responses) == '9'):
		input_requests = ''
		input_responses = ''

		if turnId % 2 == 1:
			input_requests = input_requests + first_to_play
			for item in requests:
				input_requests = input_requests + ',' + item

			head = True
			for item in responses:
				if	head:
					input_responses = input_responses + item
					head = False
				else:
					input_responses =  input_responses + ',' + item
			response = execute(input_requests, input_responses)
			#print(input_requests)
			responses.append(response)
		else:
			head = True
			#print (responses)
			for item in responses:
				if	head:
					input_requests = input_requests + item
					head = False
				else:
					input_requests =  input_requests + ',' + item

			head = True
			for item in requests:
				if	head:
					input_responses = input_responses + item
					head = False
				else:
					input_responses = input_responses + ',' +  item

			response = execute(input_requests, input_responses)

			requests.append(response)

		turnId = turnId + 1
		#print(requests)
		#print(responses)

	if (judge(input_requests, input_responses) == '1'):
		winner = 1
	else:
		winner = 0
	#return batch_size
	return turnId-2 , winner

def migrate_file(path):
	input_path = path + 'cnnInput'
	output_path = path + 'cnnOut'
	historyInput_path = path + 'historyInput_path'
	historyOutput_path = path + 'historyOutput_path'

	infile = open(input_path,'r')
	outfile = open(output_path,'r')

	fin = open(historyInput_path,'a')
	fout = open(historyOutput_path,'a')

	fin.write(infile.read())
	fout.write(outfile.read())
	
	fin.close()
	fout.close()
	infile.close()
	outfile.close()
	
	infile = open(input_path,'w')
	outfile = open(output_path,'w')

	infile.truncate()
	outfile.truncate()
	infile.close()
	outfile.close()
	
def main():
    batch_size , winner = play_once()
    print(batch_size)
    print(winner)
    #CNN_Reversi.main(batch_size=batch_size,winner=winner)
    low_level_CNNReversi.train_test(winner=winner,batch_size= batch_size,steps=500)
    migrate_file(path = path)

if __name__ == '__main__':
	main()